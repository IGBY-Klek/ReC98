public _pause
_pause  proc near
    push    bp
    mov     bp, sp
    push    si
    xor     si, si                  ; si = 0 (默认选中：再開)
    jmp     @@inp_reset_sense_for_start

; --- 输入预处理 ---
@@inp_reset_sense_for:
    call    input_reset_sense_interface

@@inp_reset_sense_for_start:
    cmp     _key_det, INPUT_NONE
    jnz     @@inp_reset_sense_for

    ; --- 初始静态菜单显示 ---
    ; 标题 (第12行)
    call    gaiji_putsa pascal, (26 shl 16) + 12, ds, offset gsCHUUDAN, TX_YELLOW
    ; 选项 (13, 14, 15行)
    call    gaiji_putsa pascal, (26 shl 16) + 13, ds, offset gsSAIKAI, TX_WHITE + TX_UNDERLINE ; 初始高亮项
    call    gaiji_putsa pascal, (26 shl 16) + 14, ds, offset gsSHUURYOU, TX_YELLOW
    call    gaiji_putsa pascal, (26 shl 16) + 15, ds, offset gsSAISHO, TX_YELLOW

@@menu_loop_start:
    call    @input_wait_for_change$qi pascal, 0

    ; --- 方向键处理 ---
    test    _key_det.lo, low INPUT_UP
    jnz     @@move_up
    test    _key_det.lo, low INPUT_DOWN
    jnz     @@move_down
    jmp     @@not_move

@@move_up:
    dec     si
    jns     @@redraw                ; si >= 0 则跳转
    mov     si, 2                   ; 循环到最后意向
    jmp     @@redraw

@@move_down:
    inc     si
    cmp     si, 3
    jb      @@redraw                ; si < 3 则跳转
    xor     si, si                  ; 循环回 0
    ; 顺延进入 @@redraw

; --- 统一重绘逻辑 ---
@@redraw:
    ; 先全部重置为普通黄色
    call    gaiji_putsa pascal, (26 shl 16) + 13, ds, offset gsSAIKAI, TX_YELLOW
    call    gaiji_putsa pascal, (26 shl 16) + 14, ds, offset gsSHUURYOU, TX_YELLOW
    call    gaiji_putsa pascal, (26 shl 16) + 15, ds, offset gsSAISHO, TX_YELLOW

    ; 根据 si 的值选择性高亮
    cmp     si, 0
    je      @@draw_0
    cmp     si, 1
    je      @@draw_1
    ; 否则是 2
    call    gaiji_putsa pascal, (26 shl 16) + 15, ds, offset gsSAISHO, TX_WHITE + TX_UNDERLINE
    jmp     @@menu_loop_start

@@draw_0:
    call    gaiji_putsa pascal, (26 shl 16) + 13, ds, offset gsSAIKAI, TX_WHITE + TX_UNDERLINE
    jmp     @@menu_loop_start

@@draw_1:
    call    gaiji_putsa pascal, (26 shl 16) + 14, ds, offset gsSHUURYOU, TX_WHITE + TX_UNDERLINE
    jmp     @@menu_loop_start

; --- 确认与退出逻辑 ---
@@not_move:
    ; 快捷键 R -> 最初 (si=2)
    test    _key_det.hi, high INPUT_R
    jz      @@check_q
    mov     si, 2
    jmp     @@menu_loop_end

@@check_q:
    ; 快捷键 Q -> 終了 (si=1)
    test    _key_det.hi, high INPUT_Q
    jz      @@check_confirm
    mov     si, 1
    jmp     @@menu_loop_end

@@check_confirm:
    ; Cancel键直接回到“再开”并退出循环
    test    _key_det.hi, high INPUT_CANCEL
    jz      @@check_shot
    mov     si, 0
    jmp     @@menu_loop_end

@@check_shot:
    ; 确认键 (SHOT 或 OK)
    test    _key_det.lo, low INPUT_SHOT
    jnz     @@menu_loop_end
    test    _key_det.hi, high INPUT_OK
    jnz     @@menu_loop_end
    jmp     @@menu_loop_start       ; 啥也没按，继续循环

@@menu_loop_end:
    ; 这里的等待逻辑：确保按键已释放，防止选完后直接触发下一个界面的输入
    call    input_reset_sense_interface
    if GAME eq 5
        test    _key_det.hi, high INPUT_CANCEL
    else
        cmp     _key_det, INPUT_NONE
    endif
    jnz     @@menu_loop_end         ; 如果按键还没松开，继续等

    ; --- 清理菜单 UI ---
    call    text_putsa pascal, (26 shl 16) + 12, ds, offset _aGAME_PAUSE_SPACES_1, TX_WHITE
    call    text_putsa pascal, (26 shl 16) + 13, ds, offset _aGAME_PAUSE_SPACES_1, TX_WHITE
    call    text_putsa pascal, (26 shl 16) + 14, ds, offset _aGAME_PAUSE_SPACES_2, TX_WHITE
    call    text_putsa pascal, (26 shl 16) + 15, ds, offset _aGAME_PAUSE_SPACES_3, TX_WHITE

    mov     ax, si                  ; 返回值：0, 1, 或 2

@@ret:
    pop     si
    pop     bp
    retn
_pause  endp