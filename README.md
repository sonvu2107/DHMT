# Mo phong Cau truc (Crane Simulation)

Du an mo phong cau truc xay dung bang **C++**, **OpenGL**, **freeglut** va **texture mapping**.

## Tinh nang

- Mo hinh cau truc tower crane: de quay, cot, buong dieu khien, can cau, cap va moc
- Texture procedural (kim loai, be tong, caro) + ho tro load file PNG/JPG
- Anh sang va vat lieu co ban
- Dieu khien tu ban phim va chuot

## Dieu khien

| Phim | Chuc nang |
|------|-----------|
| `A` / `D` hoac mui ten trai/phai | Xoay cot cau |
| `W` / `S` hoac mui ten len/xuong | Nang / ha can cau |
| `Q` / `E` | Rut / tha cap |
| `J` / `L` | Di chuyen xe con tren can |
| `R` | Reset ve trang thai ban dau |
| Chuot trai + keo | Xoay camera |
| Chuot phai + keo | Zoom camera |
| `ESC` | Thoat |

## Cai dat dependencies (Windows)

Du an tu dong build **freeglut** tu source tai `D:/freeglut-3.4.0` (co the doi bang `-DFREEGLUT_ROOT=...`).

### Build bang Visual Studio 2026

```powershell
cd C:\Users\ADMIN\Desktop\KTDH

cmake -B build -G "Visual Studio 18 2026" -A x64 "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
cmake --build build --config Release

# Chay
.\build\Release\crane_sim.exe
```

### Hoac dung vcpkg

```powershell
vcpkg install freeglut:x64-windows
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -DFREEGLUT_ROOT=NONEXISTENT
cmake --build build --config Release
```

## Texture tu file

Dat file PNG/JPG vao `assets/textures/ground.png` de dung lam texture mat dat. Neu khong co file, chuong trinh tu tao texture procedural.

## Cau truc thu muc

```
KTDH/
├── CMakeLists.txt
├── README.md
├── assets/
│   └── textures/        # (tuy chon) ground.png
├── include/
│   ├── crane.h
│   ├── texture.h
│   └── stb_image.h
└── src/
    ├── main.cpp         # GLUT window, camera, input
    ├── crane.cpp        # Mo hinh cau truc
    └── texture.cpp      # Tao/load texture
```

## Mo rong goi y

- Them vat ly cap (dao dong, va cham)
- Animation tu dong nang ha vat
- Load mo hinh 3D (.obj) thay vi primitive
- Chuyen sang OpenGL modern (shader, VBO/VAO)
