
# 🎮 GTA V External Trainer (CLI Version)

A **lightweight and functional external trainer for GTA V** written in C++. It applies cheats directly to the game’s memory without DLL injection, ImGui, or overlay. Perfect for learning, testing, and lightweight automation.

---

## ✅ Features

- [x] God Mode  
- [x] Infinite Ammo  
- [x] Sprint Speed x5  
- [x] Wanted Level 0

---

## ⚙️ Requirements

- Visual Studio 2019 or newer  
- Windows 10/11 (x64)  
- **GTA5.exe** (Steam version)  
- Administrator privileges (to access memory)

---

## 🛠️ How to Build

1. Open the project in Visual Studio.  
2. Make sure to use the `Release x64` configuration.  
3. Build the project (`Ctrl + Shift + B`).  
4. Run the generated `.exe` **as Administrator**.

---

## ▶️ How to Use

1. Launch **GTA V** and load into **Story Mode**.  
2. Run `GTA_Trainer_CLI.exe` as Administrator.  
3. Cheats will be applied automatically every 500ms.

---

## ⚠️ Notes

- The used offsets are based on **GTA V 1.67 (Steam)**. You may need to update them for future game versions.
- This project is **strictly for educational purposes**. Do **not** use it in online environments.

---

## 📂 Project Structure

```
GTA_Trainer_CLI/
├── ExternalTrainer.cpp
├── GTA_Trainer_CLI.vcxproj
├── README.md
└── WorldPTR_offset.txt  (explains 0x23D6330 origin)
```

---

## 📜 License

This project is intended for research and educational use only.  
The author **takes no responsibility** for misuse of this code.

