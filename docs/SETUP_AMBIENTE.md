# Setup do Ambiente

**Atualizado após inspeção real da máquina.** A versão anterior deste documento
estava errada: a engine **já está instalada**.

---

## 1. Estado real

| Componente | Status |
|---|---|
| Windows 11 Pro (build 26300) | OK |
| AMD Ryzen 7 5700X · 8C/16T | OK |
| 32 GB RAM | OK |
| RTX 4060 8 GB · driver 621.01 | OK |
| **Unreal Engine 5.8.4** | **Instalada em `D:\UE_5.8`** |
| MetaHuman Character (Creator in-engine) | **Presente** (6,4 GB de conteúdo) |
| OBS Studio | Instalado |
| Node.js 24.18.0 · Git 2.49 | OK |
| **Windows SDK** | **AUSENTE** |
| **MSVC x64 (Hostx64)** | **AUSENTE** |

### O que o compilador disse

```
Platform Win64 is not a valid platform to build. SDK validation failed:
  Sdk: not found. Required version 10.0.19041.0.
```

Traduzindo: falta o **Windows SDK**. Além disso, o MSVC instalado
(`14.51.36231`, dentro do *Visual Studio Build Tools 2026*) só tem o conjunto
**x86** — não tem `Hostx64/cl.exe`, que é o que a engine usa.

A engine exige (`Engine/Config/Windows/Windows_SDK.json`):

| Requisito | Versão |
|---|---|
| Windows SDK principal | 10.0.22621.0 |
| Windows SDK mínimo | 10.0.19041.0 |
| MSVC mínimo | 14.38.33130 |

---

## 2. Correção (precisa de administrador)

Dois caminhos. O primeiro é mais rápido.

### Opção A — Interface (mais simples)

1. Abra o **Visual Studio Installer**.
2. Em *Ferramentas de Build do Visual Studio 2026*, clique em **Modificar**.
3. Marque **Desenvolvimento de área de trabalho com C++**.
4. Em *Componentes individuais*, confirme:
   - `MSVC v143/v14x – build tools x64/x86` (o que traz o **Hostx64**)
   - `Windows 11 SDK (10.0.22621.0)`
5. Instalar. Cerca de 6–9 GB.

### Opção B — Linha de comando (PowerShell como Administrador)

```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe" modify `
  --installPath "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools" `
  --add Microsoft.VisualStudio.Workload.VCTools `
  --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
  --add Microsoft.VisualStudio.Component.Windows11SDK.22621 `
  --includeRecommended --passive --norestart
```

Se a 22621 não estiver disponível no catálogo, use
`Microsoft.VisualStudio.Component.Windows11SDK.26100` — a engine aceita qualquer
SDK entre 19041 e 9.99999.

### Verificação

```powershell
# deve imprimir um caminho
& 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe' -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath

# deve listar 10.0.22621.0 (ou 26100)
Get-ChildItem 'C:\Program Files (x86)\Windows Kits\10\Include' | Select-Object Name

# deve existir
Get-ChildItem 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe'
```

### Se o MSVC 14.51 for rejeitado

A engine prefere `14.50.x` (VS2026 18.0) ou `14.44.x` (VS2022 17.14). O 14.51
passa no mínimo exigido, mas se o UBT reclamar, instale o toolset exato:

```powershell
--add Microsoft.VisualStudio.Component.VC.14.50.18.0.x86.x64
```

---

## 3. Compilar

```powershell
& 'D:\UE_5.8\Engine\Build\BatchFiles\Build.bat' LiveBeatdownEditor Win64 Development `
  -Project='C:\Projetos\LiveBeatdown\Game\LiveBeatdown.uproject' -WaitMutex
```

Primeira compilação: 5–15 min (shaders não entram aqui, só C++).

## 4. Abrir

```powershell
& 'D:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\Projetos\LiveBeatdown\Game\LiveBeatdown.uproject'
```

Na primeira abertura, habilite os plugins **MetaHuman** e **MetaHuman Creator**
(`Edit → Plugins`) e reinicie.

---

## 5. Gasto de disco

| Item | Espaço |
|---|---|
| UE 5.8 já instalada | ~60 GB (em D:) |
| MetaHuman Core Data | 6,4 GB (já incluso) |
| Correção do toolchain | 6–9 GB |
| Projeto + DDC | 15–40 GB (cresce) |

Livre hoje: **C: 30,1 GB · D: 109 GB**. Cabe, mas o DDC vai crescer. Se apertar:

```powershell
setx UE_LocalDDC "D:\Epic\DDC"
```

---

## 6. Configuração de performance já aplicada

`Game/Config/DefaultEngine.ini` já traz o orçamento de VRAM:

- sem Lumen, sem ray tracing, sem reflexos dinâmicos
- sombra virtual desligada
- `t.MaxFPS=30`, VSync off
- pool de texturas em 5 GB
- sombra de fios de cabelo desligada
- `r.ScreenPercentage=100`
