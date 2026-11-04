# Arquitetura

Complementa `ANALISE_TECNICA.md` (o "porquê") com o "como". Define contratos que
**não mudam** sem registro de decisão.

---

## 1. Estrutura do repositório

```
LiveBeatdown/
├─ docs/
│  ├─ ANALISE_TECNICA.md
│  ├─ SETUP_AMBIENTE.md
│  └─ ARQUITETURA.md
├─ config/                  # fonte da verdade, editável pelo painel
│  ├─ gifts.json            # presente → golpe
│  ├─ attacks.json          # catálogo de golpes
│  ├─ tuning.json           # fila, cooldown, spam, dano
│  └─ scene.json            # câmera, luz, efeitos
├─ Bridge/                  # processo 1 — Node + TypeScript
│  ├─ src/
│  │  ├─ connectors/        # tiktoklive | eulerstream | simulator
│  │  ├─ normalize/
│  │  ├─ policy/
│  │  ├─ store/
│  │  ├─ admin/             # REST + WS
│  │  └─ server.ts
│  └─ package.json
├─ Admin/                   # painel React (build servido pelo Bridge)
│  └─ src/
└─ Game/                    # projeto Unreal
   ├─ LiveBeatdown.uproject
   └─ Source/
      ├─ LBCore/            # tipos, eventos, config, log
      ├─ LBCharacter/       # MetaHuman actor, anim BP, face
      ├─ LBCombat/          # golpes, montagens, janelas de hit
      ├─ LBPhysics/         # ragdoll, físico
      ├─ LBDamage/          # estado + materiais + malhas de dano
      ├─ LBEvents/          # receptor, fila, diretor
      ├─ LBAudio/
      ├─ LBUI/
      ├─ LBMatch/           # ciclo de rodada, estatísticas
      └─ LBAdmin/           # ponte com o Bridge
```

O jogo **não** referencia TikTok em nenhum lugar. Nem nomes, nem tipos.

---

## 2. Dependências entre módulos (UE)

```
LBCore  (nada depende de nada)
  ├─ LBCharacter
  ├─ LBCombat ──> LBPhysics, LBDamage, LBAudio
  ├─ LBDamage
  ├─ LBEvents ──> LBCombat, LBMatch
  ├─ LBUI     ──> LBDamage, LBMatch
  └─ LBMatch  ──> LBCharacter, LBDamage
```

Regra: **nunca** dependência circular. `LBCore` não conhece ninguém.
`LBEvents` não conhece `LBUI` — emite eventos e quem quiser escuta.

---

## 3. Contrato `LiveEvent`

Fronteira única entre bridge e jogo. Versionado (`v`).

```jsonc
{
  "v": 1,
  "id": "evt_01J...",          // idempotência: jogo descarta duplicado
  "kind": "gift",              // gift | comment | like | follow | join
  "ts": 1793700000000,         // epoch ms (recebimento no bridge)
  "user": {
    "id": "123",
    "uniqueId": "fulano",
    "nickname": "Fulano",
    "avatar": "https://..."
  },
  "gift": {
    "id": "5655",
    "name": "Rose",
    "diamondCount": 1,
    "count": 5,                // unidades neste evento
    "comboId": "abc123",       // presente + usuário
    "comboEnd": true,          // streak finalizado
    "streakable": true
  },
  "comment": { "text": "soco nele" },
  "meta": { "source": "tiktoklive", "raw": null }
}
```

### Regras de contagem

- `gift.count` é o número de unidades **daquele** evento.
- Presentes com streak mandam vários eventos com `count` crescente; apenas o
  último traz `comboEnd: true`.
- O bridge **acumula** por `comboId` e emite **um** evento com `count` total no
  `comboEnd`. O jogo nunca vê 30 eventos de um combo de 5 rosas.
- Timeout de 4 s sem novo evento do mesmo `comboId` força `comboEnd` (proteção
  caso o `repeat_end` se perca).

---

## 4. Schemas de configuração

### `config/attacks.json`

```jsonc
{
  "version": 1,
  "attacks": {
    "slap": {
      "label": "Tapa",
      "montage": "AM_Slap",
      "intensity": 0.25,          // 0..1
      "damageMultiplier": 0.8,
      "primaryRegion": "face",
      "hitBone": "head",
      "impulse": 250,             // N·s aplicado no ragdoll
      "reactionTier": "light",    // light | medium | heavy | special
      "cameraShake": 0.3,
      "audio": ["impact_slap_01", "impact_slap_02"],
      "vfx": "NS_Slap",
      "cooldownMs": 250
    }
  }
}
```

### `config/gifts.json`

```jsonc
{
  "version": 1,
  "defaults": { "maxQueuedPerUser": 12, "userCooldownMs": 900 },
  "gifts": {
    "Rose":    { "attack": "bat",   "countMode": "perUnit" },
    "Heart":   { "attack": "slap",  "countMode": "perUnit" },
    "IceCream":{ "attack": "punch", "countMode": "single"  },
    "HandHearts": { "attack": "kick", "countMode": "single" },
    "Galaxy":  { "attack": "cine_special", "countMode": "single" }
  }
}
```

`countMode`:
- `perUnit` — 5 rosas = 5 pauladas individuais (requisito 9).
- `single` — N unidades = 1 execução, dano × N.

### `config/tuning.json`

```jsonc
{
  "queue": {
    "slotMs": 900,
    "maxPending": 200,
    "antiStarvationSlots": 12,
    "maxConcurrentAttacks": 1
  },
  "hype": { "threshold": 15, "cooldownMs": 20000 },
  "damage": {
    "regionFalloff": 0.35,
    "balanceDecayPerSec": 0.02,
    "stunDecayPerSec": 0.05,
    "fallThreshold": 0.75
  }
}
```

Tudo isso é **dado**, não código. O painel escreve, o jogo observa e recarrega.

---

## 5. Modelo de threads

| Thread | Responsabilidade |
|---|---|
| Rede (bridge, processo separado) | websocket TikTok, normalização, persistência |
| Receiver (UE, `FRunnable`) | lê o WebSocket local, valida, injeta em `TQueue` |
| Game thread | `EventDirector`, montagens, dano, física, UI |
| Audio thread | Metasounds |
| Render thread | render |

Regra dura: **a game thread nunca espera I/O**. O receiver só empilha.

---

## 6. `EventDirector` — algoritmo

Chamado 1× por tick na game thread, com orçamento fixo.

```
1. DRENAR a fila (máx. N eventos/tick)
2. AGREGAR por (userId, attackId) numa janela de 1500 ms
3. PRIORIZAR: intensity desc; desempate por chegada
   + anti-inanição: se um item esperou > antiStarvationSlots, sobe ao topo
4. SE attack em execução E maxConcurrent atingido → aguardar
5. SENÃO: consumir 1 item → LBCombat::Play(attack, count)
6. SE pending > hype.threshold → disparar golpe especial que CONSOME o lote
   e contabiliza `hype.consumedCount` na UI
7. ESTATÍSTICAS: incrementar contadores no LBMatch
```

**Invariante testável:** `giftsRecebidos == golpesExecutados + naFila + hypeConsumidos`.
Nenhum presente evapora. Teste automatizado cobre isso.

---

## 7. Ciclo de rodada (`LBMatch`)

```
Idle → WarmingUp → Intro → Running → Finishing → Results → Idle
```

| Estado | Ações |
|---|---|
| `WarmingUp` | pré-carrega montagens/VFX/materiais (mata stutter), compila PSOs |
| `Intro` | personagem entra, câmera estabelece, HUD anima |
| `Running` | aceita eventos, dano acumula |
| `Finishing` | fila drena com prioridade, golpe final cinematográfico |
| `Results` | total de golpes, presentes, top espectador, maior combo, duração |
| `Idle` | **único ponto** em que `ResetDamage()` pode ser chamado |

Reset fora de `Idle→Intro` é proibido por design e barrado em código.

---

## 8. Dano — contrato interno

```
FDamageState                       // serializável, por rodada
  float Face, LeftEye, RightEye, Mouth, CheekSwelling,
        Forehead, Jaw, Nose, Body, Balance, Stun, Pain;
  uint8 Flags;                     // bBlackEye, bBandage, bToothChip, bLipSplit
```

Aplicação:

```
ApplyHit(FHitResult Hit, FAttackDef Attack)
  region = MapBoneAndLocation(Hit.BoneName, Hit.ImpactPoint)
  for (r, w) in RegionsWithFalloff(region, tuning.regionFalloff):
      state[r] += attack.damage * w * gift.multiplier
  state.Stun  += attack.intensity * stunGain
  state.Balance -= attack.intensity * balanceLoss
  DamageVisuals->Apply(state)     // camada A + B + C
  Physics->ApplyImpulse(Hit.ImpactPoint, attack.impulse)
```

`DamageVisuals->Apply` é **idempotente**: chamar duas vezes com o mesmo estado
não muda nada. Isso permite reaplicar após troca de LOD ou reload.

---

## 9. Falhas e reconexão

| Falha | Comportamento |
|---|---|
| Bridge cai | Jogo mostra badge "OFFLINE", continua rodando a fila pendente |
| TikTok cai | Bridge reconecta com backoff (1s→30s), loga erro no painel |
| Evento malformado | Descartado com log; contador de erro na UI |
| Evento duplicado | Descartado por `id` (LRU de 10k ids) |
| Fila estoura `maxPending` | Não descarta: agrega no item existente do mesmo usuário |
| Processo do jogo cai | Bridge continua; reconexão automática do WS |

O painel exibe todos esses estados em tempo real.

---

## 10. Requisito de teste (Fase 6)

Cenários obrigatórios antes de qualquer live real:

1. 100 presentes em 10 s → fila íntegra, contagem fecha, sem hitch > 50 ms.
2. 20 presentes simultâneos de 10 usuários distintos → sem starvation.
3. Combo de 5 rosas → exatamente 5 pauladas, 5 contabilizadas.
4. 1 presente raro no meio de um spam → não é engolido.
5. 30 golpes seguidos → dano persiste, nada reseta.
6. Queda em cada direção → personagem levanta, não atravessa o chão.
7. 20 min contínuos → VRAM < 7,2 GB, 30 FPS, sem vazamento.
