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
| **Windows SDK** | **OK** — 10.0.22621.0 e 10.0.26100.0 |
| **MSVC x64 (Hostx64)** | **OK** — 14.51.36231 |
| **.NET Framework 4.6.2 SDK (NETFXSDK)** | **OK** — `Windows Kits\NETFXSDK\4.6.2` |

### O que o compilador dizia

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

### NETFXSDK — o segundo bloqueio (já resolvido)

Depois do SDK, o UBT para em:

```
Unable to instantiate module 'SwarmInterface': Could not find NetFxSDK install dir;
this will prevent SwarmInterface from installing.
Install a version of .NET Framework SDK at 4.6.0 or higher.
```

O Windows SDK **não** traz o .NET Framework SDK. O UBT procura, nesta ordem:
`Engine/Extras/...` → registro `Microsoft\Microsoft SDKs\NETFXSDK\{4.6.2|4.6.1|4.6}`
→ e valida a existência de `Include\um\mscoree.h`.

O jeito mais leve (≈80 MB, sem tocar no Visual Studio):

```powershell
winget install --id Microsoft.DotNet.Framework.DeveloperPack.4.6 --exact --silent
```

Verificação:

```powershell
Get-ItemProperty 'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\NETFXSDK\4.6.2' |
  Select-Object KitsInstallationFolder
Test-Path 'C:\Program Files (x86)\Windows Kits\NETFXSDK\4.6.2\Include\um\mscoree.h'  # True
```

> O pacote instala em `Windows Kits\NETFXSDK\4.6.2` (sem o `10\`). É o caminho
> que o registro aponta, então funciona.

---

## 3. Compilar

```powershell
& 'D:\UE_5.8\Engine\Build\BatchFiles\Build.bat' LiveBeatdownEditor Win64 Development `
  -Project='C:\Projetos\LiveBeatdown\Game\LiveBeatdown.uproject' -WaitMutex -NoHotReload
```

Primeira compilação: 5–15 min (shaders não entram aqui, só C++).

### Se aparecer "Unable to build while Live Coding is active"

Basta ter **qualquer** editor da engine aberto (mesmo de outro projeto) para a
checagem global de Live Coding disparar. Adicione:

```powershell
-NoHotReloadFromIDE
```

Isso ignora a checagem. É seguro quando o projeto aberto é outro — as DLLs do
nosso projeto não estão carregadas por ninguém.

### Se aparecer C1076 / C3859 ("limite de heap interno" / "páginação muito pequena")

Os compiladores paralelos estouram o limite de commit do Windows (32 GB de RAM +
3,6 GB de page file). O UBA ainda mata processos acima de 37 GB, o que produz
erros de PCH que **não** são erro de código.

Reduza o paralelismo:

```powershell
-MaxParallelActions=4
```

Se persistir, aumente o page file (`SystemPropertiesAdvanced → Desempenho →
Avançado → Memória virtual`) para 16–32 GB em D:, reinicie e compile de novo.


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
