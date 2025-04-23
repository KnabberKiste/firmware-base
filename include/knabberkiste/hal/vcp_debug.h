#pragma once

void vcp_init(long long baudrate);
void vcp_putchar(char c);
void vcp_print(const char* str);
void vcp_println(const char* str);