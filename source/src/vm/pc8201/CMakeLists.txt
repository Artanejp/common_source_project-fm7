cmake_minimum_required (VERSION 2.6)

message("* vm/pc8201")

add_library(vm_pc8201
  pc8201.cpp
  cmt.cpp
  keyboard.cpp
  lcd.cpp
  memory.cpp
)
