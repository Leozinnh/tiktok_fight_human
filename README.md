# LIVE BEATDOWN

Simulador 3D de pancadaria para TikTok LIVE. Personagem humano digital reagindo a
golpes provocados por presentes e comentários, com dano progressivo persistente,
física realista e transmissão vertical 1080×1920.

> **Personagem fictício.** Todo o conteúdo é entretenimento. Nenhuma pessoa real
> é representada.

---

## Estado

| Fase | Status |
|---|---|
| 1. Análise técnica, arquitetura, decisões | **Concluída** |
| 2. Esqueleto do projeto UE + sistemas C++ | **Concluída** |
| 3. Compilação + protótipo visual | **Aguardando toolchain** |
| 4. Golpes e física em cena | Pendente |
| 5. Dano progressivo visual | Pendente |
| 6. Integração TikTok (bridge) | Pendente — por último, conforme decidido |
| 7. UI, painel, otimização | Pendente |

## O que já existe

**Unreal Engine 5.8.4** em `D:\UE_5.8`, com MetaHuman Creator in-engine.

Projeto em `Game/` com 11 módulos compiláveis:

| Módulo | Responsabilidade |
|---|---|
| `LBCore` | Tipos canônicos, catálogo de golpes, configuração JSON |
| `LBDamage` | Estado de dano por região, material e malha de dano |
| `LBPhysics` | Ragdoll ativo em 3 estágios |
| `LBCombat` | Execução de golpe, impacto, impulso |
| `LBCharacter` | Composição do alvo (MetaHuman) |
| `LBEvents` | Fila thread-safe, receptor WebSocket, diretor de eventos |
| `LBMatch` | Ciclo de rodada, estatísticas, ranking |
| `LBAudio` | Banco de sons de impacto com variação |
| `LBUI` | Base do HUD vertical |
| `LBAdmin` | Diagnóstico em JSON para o painel |
| `LiveBeatdown` | GameMode e entrada |

Configuração externa em `Game/Config/`: `attacks.json`, `gifts.json`,
`tuning.json`. Nada de presente ou golpe hardcoded.

## Documentação

- `docs/ANALISE_TECNICA.md` — engine, personagem, dano, física, TikTok, custos, cronograma
- `docs/ARQUITETURA.md` — contratos, schemas, threads, algoritmo do diretor
- `docs/SETUP_AMBIENTE.md` — estado da máquina e como compilar

## Compilar

```powershell
& 'D:\UE_5.8\Engine\Build\BatchFiles\Build.bat' LiveBeatdownEditor Win64 Development `
  -Project='C:\Projetos\LiveBeatdown\Game\LiveBeatdown.uproject' -WaitMutex
```

Requer Windows SDK + MSVC x64 — ver `docs/SETUP_AMBIENTE.md`.

## Abrir

```powershell
& 'D:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' `
  'C:\Projetos\LiveBeatdown\Game\LiveBeatdown.uproject'
```
