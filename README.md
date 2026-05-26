# Mo phong Cau truc (GLUT thuan, khong CMake)

Project nay giong phong cach bai Ha Manh Toan: code C++ + OpenGL/GLUT chay truc tiep, khong dung CMake.

## Cau truc hien tai

- `src/main.cpp`: toan bo logic (model, render, input, texture load)
- `include/stb_image.h`: doc anh BMP/PNG/JPG
- `assets/textures/*.bmp`: texture su dung cho co, troi, than may
- `.vscode/tasks.json`: lenh build nhanh trong VS Code

## Dieu khien

- `A` / `D`: xoay cot
- `W` / `S`: nang/ha can
- `Q` / `E`: rut/tha cap
- `J` / `L`: dich xe con tren can
- `R`: reset trang thai
- Chuot trai + keo: xoay camera
- Chuot phai + keo: zoom
- `ESC`: thoat

## Build va chay (MSYS2 UCRT64)

```powershell
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -O2 -Iinclude src\main.cpp -LC:\msys64\ucrt64\lib -lfreeglut -lopengl32 -lglu32 -lwinmm -o build\crane_sim.exe
.\build\crane_sim.exe
```

## Build bang VS Code

- Dung task mac dinh trong `.vscode/tasks.json` (Ctrl+Shift+B).
- Output EXE se nam trong thu muc `build/`.
