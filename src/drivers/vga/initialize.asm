global vga_set_video_mode
vga_set_video_mode:
    mov ah, 00h
    mov al, 13h
    int 10h

global vga_set_text_mode
vga_set_text_mode:
    mov ah, 00h
    mov al, 03h
    int 10h
