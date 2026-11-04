# Setup do Ambiente

Objetivo: deixar a máquina pronta para a Fase 2 (protótipo visual).
Tudo instalado em **D:\** conforme decidido.

---

## 1. Situação atual (medida em 03/11/2026)

| Componente | Status |
|---|---|
| Windows 11 Pro | OK |
| AMD Ryzen 7 5700X · 32 GB RAM | OK |
| RTX 4060 8 GB · driver 621.01 | OK |
| Epic Games Launcher | Instalado |
| **Unreal Engine 5** | **Não instalado** |
| **Visual Studio 2022 + C++** | **Ausente** (só sobraram pastas vazias — sem `cl.exe`, sem MSBuild, sem Windows SDK) |
| OBS Studio | Instalado |
| Node.js 24.18.0 | OK |
| Git 2.49.0 | OK |

---

## 2. Matemática do disco — leia antes de instalar

Estado: **C: 21,4 GB livres · D: 109,0 GB livres**.

Consumo mínimo do projeto:

| Item | Espaço | Onde |
|---|---|---|
| UE 5.6 (sem símbolos de debug, sem source) | 45–55 GB | D: |
| MetaHuman Creator Core Data | 8–12 GB | D: |
| Visual Studio 2022 + workload C++ de jogos | 20–25 GB | D: |
| Projeto + Content + MetaHuman | 6–10 GB | D: |
| DDC (derived data — **cresce durante o uso**) | 15–40 GB | D: |
| **Total em D:** | **94–142 GB** | |
| **Livre em D: hoje** | **109 GB** | |

**Conclusão:** o mínimo cabe com aperto, o realista **não** cabe. Além disso o C:
com 21 GB livres vai sofrer com pagefile e temporários durante a compilação de
shaders — o Windows precisa de folga.

### Ação necessária

| Meta | Mínimo aceitável | Recomendado |
|---|---|---|
| Livre em D: | **140 GB** | 200 GB |
| Livre em C: | **35 GB** | 50 GB |

Ou seja: liberar **~35 GB em D:** e **~15 GB em C:**, **ou** instalar um SSD
extra. Sem isso, a instalação vai falhar no meio (e uma engine instalada pela
metade dá erro de forma confusa).

Nada aqui é contornável por software: engine e toolchain não podem ser instalados
parcialmente.

---

## 3. Instalação do Unreal Engine 5.6

1. Abra o **Epic Games Launcher** e faça login (conta gratuita).
2. **Unreal Engine → Library → +** e escolha a versão **5.6** (ou a 5.7, se já
   estiver disponível e estável — decidiremos juntos).
3. Clique em **Options** e **desmarque**:
   - `Engine Source` (economiza ~25 GB)
   - `Editor symbols for debugging` (economiza ~30 GB)
4. **Marque**:
   - `MetaHuman Creator Core Data` ← **obrigatório**. Sem isso o Creator abre com
     a edição de pele desabilitada.
5. Instale em `D:\Epic\UE_5.6`.
6. Após instalar, dentro do editor: `Edit → Plugins` → habilitar
   **MetaHuman** e **MetaHuman Creator** → reiniciar o editor.

---

## 4. Instalação do Visual Studio 2022

Sem isso não há C++, e o projeto UE nem gera os arquivos de solução.

1. Baixe **Visual Studio Community 2022** (gratuito).
2. Workload obrigatório: **Desenvolvimento de jogos com C++**.
3. Componentes individuais a confirmar:
   - `MSVC v143 – VS 2022 C++ x64/x86 build tools`
   - `Windows 11 SDK` (10.0.22621 ou superior)
   - `.NET desktop development` (para ferramentas auxiliares)
4. Instale em `D:\Microsoft Visual Studio\2022\Community`.
5. **Não** instale workloads de web/mobile — só consomem disco.

---

## 5. Pós-instalação

```powershell
# DDC e intermediates no D:, para não encher o C:
setx UE_LocalDDC "D:\Epic\DDC"
setx UE_SharedDDC "D:\Epic\DDC\Shared"
```

- DDC (Derived Data Cache) guarda shaders compilados. Fica em `D:` para não
  consumir o C:. Vai crescer: limpar periodicamente é normal.
- Pagefile: mantém no C: (padrão). Só mexa se o C: ficar sem espaço.

---

## 6. Verificação

Quando terminar, rode e me envie a saída:

```powershell
Get-ChildItem 'D:\Epic\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe' | Select-Object FullName,Length
& 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe' -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
Get-PSDrive -PSProvider FileSystem | Select-Object Name,@{n='FreeGB';e={[math]::Round($_.Free/1GB,1)}}
```

Esperado: caminho do editor, caminho do VS, e livre em D: ≥ 40 GB após instalar.

---

## 7. O que eu faço em paralelo

Enquanto o download roda (1–3 h), posso adiantar sem depender da engine:

- estrutura do repositório (`.gitignore`, pastas de módulos);
- **bridge Node/TypeScript** com os adaptadores e o simulador local — é
  totalmente testável sem TikTok e sem UE;
- contrato de eventos e esquemas de configuração (`gifts.json`, `attacks.json`);
- `docs/ARQUITETURA.md` com o mapa de módulos e o fluxo de dados.

Isso significa que, quando a engine estiver pronta, a Fase 2 começa direto no
visual, sem perder tempo em encanamento.
