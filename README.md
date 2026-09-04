# The Touhou PC-98 Restoration Project (*"ReC98"*) — IGBY-Klek's modded fork

This is a personal fork of the [ReC98](https://github.com/nmlgc/ReC98) debloated source tree, with a set of gameplay and quality-of-life modifications for **TH02 (封魔録 / the Story of Eastern Wonderland)**, **TH04 (幻想郷 / Lotus Land Story)** and **TH05 (怪綺談 / Mystic Square)**.

All modifications are compiled from the ReC98 sources in this repository and are intended for the original NEC PC-9801 versions of the games (tested under [Neko Project 21/W](https://www.yui.ne.jp/np21/), on a 640×400 high-resolution build with the `D0903.hdi` environment).

> The upstream ReC98 project aims to perfectly reconstruct the source code of the first five Touhou games, such that binaries compiled from this code are indistinguishable from ZUN's original builds. This fork intentionally deviates from that goal: the code here contains visible, deliberate modifications. See the upstream [README](https://github.com/nmlgc/ReC98) and [homepage](https://rec98.nmlgc.net) for the unmodified project.

---

## TH04 / TH05 (shared code)

| Feature | Implementation |
|---|---|
| **Hitbox display on Shift** | While Shift is held, a 16×16 hitbox sprite is drawn centered on the player, using the unused item-slot cel `PAT_ITEM + 7` of `miko16.bft` (TH04: patnum 51, TH05: 43), rendered via `z_super_roll_put_tiny_16x16_raw` in the shared `th04/main/player/render.asm` (`PLAYER_HITBOX_PATNUM`). |
| **FPS counter** (bottom-left, row 23) | `fps_counter_update()` in `th04/main/hud/overlay.cpp`, called once per rendered frame from `th04_main.asm` / `th05_main.asm`. Measured against master.lib's 60 Hz `vsync_Count2`; displayed as `00.00 FPS` on the text plane. |
| **Point-of-collection fixed during boss fights** | The TH04/TH05 pull switch (`items_pull_to_player`) is now latched and forcibly re-evaluated at the top of every `items_update()` call, so items keep being collected while the player stays in the top quarter of the playfield during boss battles. |
| **Kurumi / Marisa-4 divide-by-zero crash** | Fixed. |
| **Pause menu: restart** | Pressing **R** in the pause menu exits to the OP with a marker in `resident.demo_num` (0xFF = main game, 0xFE = Extra). The OP's `main()` detects the marker and immediately calls the new `start_game_auto()` / `start_extra_auto()` (OP entry points without the shottype selection), starting a fresh run in stage 1 / the Extra stage. |
| **TH05 GJINIT font** | Pause-screen kanji are loaded from `th05/sprites/gaiji.bmp` → `gaiji.asp` → `gjinit.com`. Note: `bmp2arr` outputs sprite **N** from BMP slot **N−1**, so glyphs for pause codes F6/F7 must be drawn into BMP slots 245/246. |

## TH02 (封魔録)

| Feature | Implementation |
|---|---|
| **Hitbox shrunk to center ±3** | Player collision box narrowed (2 checks in `bullet.cpp` / `player.cpp`). |
| **Hitbox display on Shift** | While Shift is held, the hitbox is shown as the smallest bullet cel (`PAT_BULLET16_BALL`). |
| **Shottype screen: Esc returns to the main menu** | `shottype_menu()` returns `false` on Esc; returning to the main menu re-uses the OP background loading logic (menu backdrop restored with `PaletteTone = 100` first, `pi_free()` moved to the confirm path only). |
| **Quitting no longer shows the scoreboard / continue screen** | Esc in the pause menu sets `resident.unused_1`; `_main` skips the game-over flow and `execl`s straight back to the OP. |
| **Point-of-collection** (TH04/TH05 style: top quarter of the playfield, or while bombing) | Implemented with **zero added global state**: the per-item `age` field doubles as the pull latch (1 = pulled), and the pull condition is a function-local flag. Pulled items home in on the player center at 7 px/frame. |
| **FPS counter** (bottom-left, row 23) | Same design as TH04/TH05, but the state lives in the two unused bytes of `_scroll_unused`, and the label is rendered in **full-width SJIS glyphs** (`００.００ ＦＰＳ`) — see the technical notes below. |
| **Pause menu: Traditional Chinese + "restart"** | The pause menu is now `暫停 / 繼續遊戲 / 從頭開始 / 結束遊戲`, with the original fan-translation confirm lines (`真的～要結束遊戲龜? / 騙齶的齬,不退不退。 / 是真的黷,退齷退齷。`). Selecting 從頭開始 sets `resident.unused_2` (1 = normal, 2 = Extra, detected via `_stage_id == 5`) and leaves through the regular manual-quit flow; the OP's main loop spots the marker and auto-starts a new run via `start_game_auto()` / `start_extra_auto()`, keeping the current shottype and options. |
| **Link fix: `vector2()` made far** | The byte-exact near-call hack in `spark.cpp` overflows once the code grows; the function is now called through the regular (far) declaration. |

## Technical notes for maintainers (hard-won lessons)

These quirks of the TH02 main executable drove most of the implementation constraints:

1. **TH02 `main.exe` is allergic to any growth of its DGROUP.** Adding even *one* byte of data to `_DATA` or `_BSS` (e.g. a single unused global) makes the game corrupt enemy hit-testing, clear entire stages of enemies, or crash on startup — presumably due to hardcoded segment-address assumptions in the original binary. All TH02 state therefore **reuses existing fields** (`age`, `_scroll_unused`, `resident.unused_1/2`), and all strings are kept in code segments.
2. **`to_sp()` with a non-constant argument pulls in the whole 8087 emulation runtime** (+~17.7 KB in `_TEXT`), which breaks the game. Use the integer macro `TO_SP()` for runtime values.
3. **Writing half-width ASCII to text row 23 via `text_putsa()` mirrors the game's gaiji note glyphs** on that row (BGM indicator). Render the FPS counter with full-width SJIS glyphs instead.
4. **master.lib text-plane address:** on the 640×400 high-resolution mode the text VRAM lives at **`0xE0000`** (`TextVramSeg`), not `0xA0000`.
5. The TH04/TH05 point-collection fix relies on `_items_pull_to_player` (byte) plus the per-item `pulled_to_player` latch, refreshed at the top of `items_update()`.

## Data/asset fixes

* **TH04.DAT repack corruption:** a previous repack (adding the NAME track and `SCNUM.BFT`) wrote an empty **`BB0.BB`** entry (zsize/size/offset = 0), which broke the bomb portrait BB animation. The entry is restored from the known-good archive; `BB0.CDG` was unaffected.

## Building

* DevKit: `D:\ReC98DevKit` (Turbo C++ 4.0J, TASM 5.0, tup)
* `build.bat` → `tup` incremental build; TH02 outputs: `bin/th02/main.exe`, `bin/th02/op.exe`
* Debugging: Neko Project 21/W debug build with the LLM bridge (`np21llm_cmd.txt` / `np21llm_out.txt`), disk image `D0903.hdi`.

## Author

**IGBY-Klek** — Keunakhan@outlook.com

Repository: https://github.com/IGBY-Klek/ReC98 (branch `main`)

All gameplay modifications are original work layered on top of the ReC98 source; upstream ReC98 licensing applies to the base code.

---

# 简体中文翻译

## 东方 PC-98 复原工程 (*"ReC98"*) —— IGBY-Klek 修改版

这是 [ReC98](https://github.com/nmlgc/ReC98) 去膨胀(debloated)源码树的个人 fork,为 **TH02(封魔録 / the Story of Eastern Wonderland)**、**TH04(幻想郷 / Lotus Land Story)** 与 **TH05(怪綺談 / Mystic Square)** 添加了一系列玩法与便利性修改。

所有修改均基于本仓库中的 ReC98 源码编译,面向 NEC PC-9801 原版游戏(已在 [Neko Project 21/W](https://www.yui.ne.jp/np21/) 上验证,640×400 高分辨率环境,`D0903.hdi` 磁盘镜像)。

> 上游 ReC98 的目标是完美复原前五作东方游戏源码,使由此代码编译出的二进制与 ZUN 原版无法区分。本 fork 有意偏离该目标:这里的代码包含可见的、有意的修改。未修改版本请参阅上游 [README](https://github.com/nmlgc/ReC98) 与[主页](https://rec98.nmlgc.net)。

---

### TH04 / TH05(共享代码)

| 功能 | 实现 |
|---|---|
| **按住 Shift 显示判定点** | 按住 Shift 时,以自机为中心绘制 16×16 判定点,使用 `miko16.bft` 中未使用的道具槽 cel `PAT_ITEM + 7`(TH04:patnum 51,TH05:43),在共享的 `th04/main/player/render.asm` 中通过 `z_super_roll_put_tiny_16x16_raw` 渲染(`PLAYER_HITBOX_PATNUM`)。 |
| **FPS 计数器**(左下角,第 23 行)| `th04/main/hud/overlay.cpp` 中的 `fps_counter_update()`,由 `th04_main.asm` / `th05_main.asm` 每渲染帧调用一次。以 master.lib 的 60Hz `vsync_Count2` 为基准,以 `00.00 FPS` 显示在文本层。 |
| **修复 Boss 战期间收点失效** | TH04/TH05 的收点开关(`items_pull_to_player`)现在会被锁存,并在每次 `items_update()` 开头强制重算,使玩家在 Boss 战期间停留在画面顶部四分之一区域时道具仍持续被收集。 |
| **Kurumi / Marisa-4 除零崩溃** | 已修复。 |
| **暂停菜单:重新开始** | 在暂停菜单中按 **R** 会携带标记退出到 OP(`resident.demo_num`:0xFF = 主线,0xFE = Extra)。OP 的 `main()` 检测到标记后立即调用新增的 `start_game_auto()` / `start_extra_auto()`(去掉选人界面的 OP 启动入口),直接开始第 1 面或 Extra 面的新一局。 |
| **TH05 GJINIT 字体** | 暂停界面汉字由 `th05/sprites/gaiji.bmp` → `gaiji.asp` → `gjinit.com` 加载。注意:`bmp2arr` 输出的 sprite **N** 对应 BMP 第 **N−1** 格,因此暂停代码 F6/F7 的字形必须画在 BMP 第 245/246 格。 |

### TH02(封魔録)

| 功能 | 实现 |
|---|---|
| **判定框缩小至中心 ±3** | 自机碰撞框收窄(`bullet.cpp` / `player.cpp` 两处判断)。 |
| **按住 Shift 显示判定点** | 按住 Shift 时以最小子弹 cel(`PAT_BULLET16_BALL`)显示判定点。 |
| **选人界面:Esc 返回主菜单** | `shottype_menu()` 在 Esc 时返回 `false`;返回主菜单复用 OP 背景加载逻辑(恢复背景前先将 `PaletteTone = 100`,`pi_free()` 仅保留在确认路径)。 |
| **退出游戏不再显示计分板 / 续关界面** | 暂停菜单按 Esc 设置 `resident.unused_1`;`_main` 跳过游戏结束流程直接 `execl` 回 OP。 |
| **收点**(TH04/TH05 式:画面顶部四分之一,或放炸弹期间)| 以**零新增全局状态**实现:道具自身的 `age` 字段兼作吸附锁存(1 = 被吸附),吸附条件为函数局部变量。被吸附道具以 7 像素/帧的速度直线飞向自机中心。 |
| **FPS 计数器**(左下角,第 23 行)| 设计与 TH04/TH05 相同,但状态存放在 `_scroll_unused` 的两个未用字节中,且文字以**全角 SJIS 字形**(`００.００ ＦＰＳ`)绘制 —— 原因见下文技术笔记。 |
| **暂停菜单:繁体中文 + "重新开始"** | 暂停菜单现为 `暫停 / 繼續遊戲 / 從頭開始 / 結束遊戲`,确认框沿用民间汉化版原句(`真的～要結束遊戲龜? / 騙齶的齬,不退不退。 / 是真的黷,退齷退齷。`)。选择 從頭開始 会设置 `resident.unused_2`(1 = 主线,2 = Extra,以 `_stage_id == 5` 判定)并走常规手动退出流程;OP 主循环发现标记后经 `start_game_auto()` / `start_extra_auto()` 自动开始新一局,保留当前角色与选项。 |
| **链接修复:`vector2()` 改为 far** | `spark.cpp` 中逐字节精确的 near 调用在代码增长后会溢出;该函数现通过常规(far)声明调用。 |

### 维护者技术笔记(来之不易的教训)

以下 TH02 主程序的可执行文件怪癖决定了大部分实现约束:

1. **TH02 `main.exe` 对 DGROUP 的任何增长都过敏。** 只要向 `_DATA` 或 `_BSS` 增加一个字节数据(哪怕一个未使用的全局变量),游戏就会出现敌人碰撞错乱、整面敌人消失或启动崩溃 —— 推测是原二进制中存在硬编码段地址假设。因此 TH02 的所有状态都**复用现有字段**(`age`、`_scroll_unused`、`resident.unused_1/2`),所有字符串都放在代码段中。
2. **`to_sp()` 传入非常量参数会引入整个 8087 仿真运行库**(`_TEXT` 增加约 17.7KB),导致游戏损坏。运行期数值请改用整数宏 `TO_SP()`。
3. **通过 `text_putsa()` 向第 23 行写入半角 ASCII 会使该行的 gaiji 音符图形(BGM 指示器)镜像显示。** FPS 计数器应改用全角 SJIS 字形。
4. **master.lib 文本层地址:** 640×400 高分辨率模式下,文本 VRAM 位于 **`0xE0000`**(`TextVramSeg`),而非 `0xA0000`。
5. TH04/TH05 的收点修复依赖 `_items_pull_to_player`(字节)与逐道具的 `pulled_to_player` 锁存,并在 `items_update()` 开头刷新。

### 数据 / 资源修复

* **TH04.DAT 重打包损坏:** 之前的一次重打包(加入 NAME 曲与 `SCNUM.BFT`)把 **`BB0.BB`** 条目写成了空(zsize/size/offset 均为 0),导致炸弹立绘 BB 动画损坏。该条目已从已知完好档案恢复;`BB0.CDG` 未受影响。

### 构建

* DevKit:`D:\ReC98DevKit`(Turbo C++ 4.0J、TASM 5.0、tup)
* `build.bat` → `tup` 增量构建;TH02 产物:`bin/th02/main.exe`、`bin/th02/op.exe`
* 调试:Neko Project 21/W 带 LLM 桥的调试版(`np21llm_cmd.txt` / `np21llm_out.txt`),磁盘镜像 `D0903.hdi`。

### 作者

**IGBY-Klek** — Keunakhan@outlook.com

仓库:https://github.com/IGBY-Klek/ReC98 (分支 `main`)

所有玩法修改均为叠加于 ReC98 源码之上的原创工作;基础代码适用上游 ReC98 许可证。
