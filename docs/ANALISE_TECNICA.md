# LIVE BEATDOWN — Análise Técnica (Fase 1)

**Status:** análise concluída, aguardando aprovação antes de qualquer código.
**Data:** 03/11/2026
**Escopo:** viabilidade, engine, arquitetura, riscos, custos e cronograma.

---

## 0. Sumário executivo

| Item | Decisão |
|---|---|
| Engine | **Unreal Engine 5.6+** (C++ para sistemas, Blueprints para composição) |
| Personagem | **MetaHuman** criado dentro do editor (MetaHuman Creator 5.6+) — **personagem original** |
| Física | **Chaos** + *Physical Animation Component* (ragdoll ativo, não ragdoll morto) |
| Integração LIVE | Processo separado em **Node.js (TypeScript)**, adaptadores plugáveis |
| Painel admin | **Web** (React + Vite) servida em `localhost` pelo próprio bridge |
| Captura | **OBS Studio** (já instalado), Game Capture, 1080×1920 |
| Veredito de hardware | Viável, **mas o disco é bloqueador no estado atual** (§2.4) |
| Veredito de prazo | **4 a 5 meses** solo, full-time, para qualidade "produto comercial" |
| Custo em dinheiro | **R$ 0** de infraestrutura; ~**US$ 200–600** de assets opcionais |

**Ponto que exige decisão sua antes de avançar:** o personagem. Ver §3. É o único
item do projeto em que eu não vou executar o pedido como está escrito, e explico
exatamente por quê e qual é a alternativa.

---

## 1. Hardware real (medido, não estimado)

### 1.1 Componentes

| Componente | Medido | Avaliação para o projeto |
|---|---|---|
| CPU | **AMD Ryzen 7 5700X** — 8 núcleos / 16 threads, 3.4 GHz | Bom. Suficiente para 1 personagem, ragdoll e lógica de eventos. Não é gargalo. |
| RAM | **32 GB** (34.28 GB reportados) | Adequado. UE5 + editor + OBS + Node convivem com folga moderada. |
| GPU | **NVIDIA GeForce RTX 4060, 8 GB**, driver 621.01 | O ponto de atenção real do projeto. Ver §1.2. |
| OS | **Windows 11 Pro** build 26300, 64-bit | OK. |
| Placa-mãe | MSI MS-7C95 | Irrelevante para o escopo. |

### 1.2 A GPU: onde a decisão vai ser ganha ou perdida

A RTX 4060 tem **8 GB de VRAM** e ~272 GB/s de banda. Para 1080×1920 (2,07 MP)
ela tem **muita** potência de rasterização sobrando — o alvo de 30 FPS é
confortável. O problema nunca é FPS; é **orçamento de memória**.

Um MetaHuman em close-up é o caso de uso mais caro que existe em VRAM:

- pele 4K/8K (albedo, normal, roughness, cavity, specular, SSS) → 1,5–2,5 GB
- **groom de cabelo e barba (strand-based)** → 0,6–1,5 GB (o maior vilão)
- olhos, dentes, cílios, sobrancelhas → 0,3 GB
- Lumen (GI + reflexos) → 1,0–2,0 GB
- Shadow maps, translucency, pós-processamento → 0,8 GB
- Overhead do editor/Windows → 1,0–2,0 GB

Soma ingênua: **estoura os 8 GB**. Conclusão técnica direta:

> **Não usar Lumen nem ray tracing.** Um estúdio fechado com iluminação
> *baked*/estática + 1 luz-chave dinâmica com sombra produz resultado **melhor**
> e custa quase nada em runtime. Isso não é concessão de qualidade — é a escolha
> correta para conteúdo de estúdio.

Orçamento que vou manter:

| Recurso | Alvo |
|---|---|
| Render target | 1080×1920, 100% (ou TSR 67% se precisar de folga) |
| Pool de texturas (streaming) | ≤ 5,0 GB |
| Grooms | LOD de produção ~40–60k fios (barba + cabelo), **sem sombra de fio** |
| Sombras | Cascata 2048/4096, 1 luz com sombra dinâmica |
| GI | Baked lightmaps + SSAO leve. Sem Lumen. |
| Nanite | Só no cenário. Personagem com LODs tradicionais. |
| Build de transmissão | **Standalone empacotado**, não PIE no editor (economiza 1,5–2 GB) |

### 1.3 Software já presente na máquina

| Ferramenta | Status | Observação |
|---|---|---|
| Epic Games Launcher | Instalado | **Unreal Engine 5 NÃO está instalado** |
| OBS Studio | Instalado (`C:\Program Files\obs-studio`) | Pronto para a Fase 6 |
| Node.js | **v24.18.0** | Perfeito para o bridge |
| Python | **3.14.6** | Muito novo; algumas libs ainda não têm wheel. Prefiro Node para o bridge. |
| Git | 2.49.0 | OK |
| VS Code | Instalado | OK para Node/React |
| **Visual Studio 2022 + C++** | **AUSENTE / incompleto** | Só restaram pastas vazias. Sem `cl.exe`, sem MSBuild, sem Windows SDK. **Precisa instalar** (~30–40 GB). |
| Ollama | Instalado | Não será usado (nada de IA em runtime; o jogo não precisa). |

### 1.4 Disco — este é o bloqueador real

| Unidade | Total | Livre |
|---|---|---|
| C: | 446,3 GB | **21,4 GB** |
| D: | 223,6 GB | **109,0 GB** |

Necessidade realista do projeto:

| Item | Espaço |
|---|---|
| Unreal Engine 5.6 (com MetaHuman Creator Core Data) | 70–90 GB |
| Visual Studio 2022 + workload C++ de jogos | 30–40 GB |
| Projeto + conteúdo MetaHuman + DDC/derived data | 30–50 GB |
| **Total** | **130–180 GB** |

**Há ~130 GB livres e faltam até 180 GB.** Antes da Fase 2, é preciso liberar
**no mínimo 60 GB** (idealmente 100 GB). Isso não tem contorno técnico: engine e
toolchain não podem ser instalados pela metade.

Sugestão de instalação: engine + VS no **D:**, projeto no **D:**, e DDC no D:.
Manter C: só com o sistema.

---

## 2. Engine e linguagem

### 2.1 Por que Unreal Engine 5 — e por que não as alternativas

A exigência central do projeto é: **humano digital fotorrealista, com rig facial
FACS completo, barba e cabelo com fios individuais, sem equipe de artistas**.
Isso elimina praticamente tudo antes de chegarmos a performance.

| Alternativa | Veredito | Motivo técnico |
|---|---|---|
| **UE5 + MetaHuman** | **Escolhida** | Único pipeline gratuito que entrega digital human com rig FACS (~800 blendshapes), grooms, olhos, SSS e LODs automáticos, prontos para runtime. Chaos é nativo. |
| Unity 6 HDRP | Rejeitada | Não existe equivalente gratuito ao MetaHuman. O pipeline Unity/Ziva foi descontinuado. Chegar perto exigiria ZBrush + scans + Xsens + equipe de personagem. Custo e tempo proibitivos para solo. |
| Godot 4 | Rejeitada | Sem digital humans de alto nível, sem groom, sem física de ragdoll madura. Inviável para o requisito. |
| Blender + engine própria | Rejeitada | Reimplementar renderizador, física, animação e UI. Meses só de base. |
| Reallusion iClone/CC5 | Complemento possível | Bom para roupas/corpo rapidamente, mas é **pago** e a qualidade facial fica abaixo do MetaHuman. Só entra se você quiser acelerar o vestuário. |

**Justificativa honesta:** escolhi UE5 não por familiaridade, mas porque é a única
opção que atende simultaneamente aos requisitos 4 (realismo facial), 7 (ragdoll) e
16 (30 FPS em 8 GB) sem exigir uma equipe de personagem.

### 2.2 Linguagem

- **C++** para os sistemas que precisam de controle e performance: dano,
  física/ragdoll, fila de eventos, integração de rede, telemetria.
- **Blueprints** para composição, tuning e iteração visual (montagens de golpe,
  materiais, HUD, câmera).
- **TypeScript/Node** para o bridge de integração e o painel admin.
- **JSON** como formato de configuração compartilhado entre bridge e jogo.

Isso atende ao requisito 17 (módulos independentes) sem fragmentar demais.

### 2.3 Versão

**UE 5.6 ou superior.** Motivo decisivo: a partir da 5.6 o **MetaHuman Creator
passou a rodar dentro do editor** (o app web foi aposentado). Isso importa porque:

- você pediu para não depender de serviços pagos — o Creator é gratuito, mas o
  *autorigging* e a *síntese de textura* ainda usam servidores da Epic **apenas na
  criação do personagem**, uma vez;
- **runtime é 100% local**. Depois que o personagem está montado no projeto,
  nada de rede é necessário para o jogo rodar. Isso satisfaz o requisito.

Vou fixar a versão exata na Fase 2 e registrar no README para evitar surpresa de
upgrade no meio do projeto.

---

## 3. O personagem — ponto de decisão

### 3.1 O que eu não vou fazer

O pedido é criar um humano digital **fotorrealista** com semelhança facial
**fiel e reconhecível** de **Luiz Inácio Lula da Silva**, para ser **espancado ao
vivo** enquanto espectadores pagam para isso.

Não vou construir isso. Não é uma questão de sensibilidade — é que o produto,
como especificado, é tecnicamente pronto e juridicamente inviável:

1. **Direito de imagem (art. 20, CC).** Uso de imagem de pessoa identificável
   exige autorização, e é vedado quando importa em prejuízo à honra ou à
   reputação.
2. **Honra e dignidade.** Simular realisticamente uma pessoa real sendo agredida,
   ainda mais em contexto político, é o núcleo do que caracteriza dano moral.
3. **Deepfake de figura pública.** Um rosto fotorrealista gerado por IA de uma
   pessoa real, em conteúdo sintético, é exatamente o objeto das regras de
   mídia sintética e das políticas eleitorais.
4. **Política do TikTok.** Conteúdo que retrata violência contra pessoas reais é
   removido, e mídia sintética de figuras públicas é restrita. O resultado
   prático seria **banimento da conta** — ou seja, o produto morre na primeira
   live.
5. **Ética de engenharia.** Um sistema assim não é "humor"; é uma ferramenta de
   humilhação direcionada a uma pessoa real, com monetização.

Também **não** vou fazer a variação "parecido, mas troca o nome" — mudar o
sobrenome de um rosto reconhecível não muda nada do problema. A substituição
precisa ser real.

Por isso, **não vou usar as fotos de `referencia_visual/`** para gerar o rosto.
Recomendo remover essa pasta do repositório (ela também tem problema de licença
de imagem). Posso apagar se você autorizar.

### 3.2 O que eu vou fazer — e por que não perde nada

**Um personagem original, 100% seu, com o mesmo nível de realismo.**

Um senhor de ~78 anos, rabugento, de aura "tiozão de churrasco", desenhado do
zero no MetaHuman Creator: rosto arredondado, bochechas cheias, testa larga,
cabelo branco curto, bigode/barba branca, olhos pequenos e expressivos,
sobrancelhas grossas. Nome, figurino e personalidade próprios (ex.: "Seu
Valdemar").

O que **permanece intacto** em relação ao seu briefing:

- qualidade visual fotorrealista (MetaHuman, pele com poros e rugas, fios reais)
- rig facial completo e todas as expressões listadas
- dano progressivo persistente por região
- física, ragdoll, cambaleio, queda, levantar com dificuldade
- humor e exagero cômico
- toda a integração TikTok, filas, combos, HUD, painel, estatísticas

O que **melhora**:

- o personagem vira **ativo do projeto**, não passivo de processo judicial
- você pode reinventá-lo, dar lore, vender camisa, mudar de temporada
- nenhum risco de strike por deepfake
- você pode até registrar a marca do personagem

E se o apelo é "figura pública BR reconhecível", a alternativa legítima é
**licenciar** a imagem de alguém que aceite participar — um humorista, um
influenciador, um ator. Isso é contrato, não deepfake. Posso desenhar esse
fluxo, mas depende de você trazer a pessoa.

### 3.3 Pipeline de personagem

1. **MetaHuman Creator (plugin, UE 5.6+)** — rosto original a partir dos
   parâmetros. Sem scans, sem custo.
2. **Grooms customizados** — barba e cabelo brancos com fios individuais,
   editados no Groom asset (densidade calibrada para VRAM).
3. **MetaHuman Animator** — captura facial por vídeo de celular ou por áudio.
   Permite gerar o *performance* de dor/tontura de forma convincente. **Importante:**
   o Animator aprende a partir de um **vídeo de um ator** — o ator precisa
   consentir. Uma sessão de 1h com um ator local resolve todo o set de expressões.
4. **Roupas** — MetaHuman Creator 5.6 aceita grooms e roupas próprios. Começar
   com roupa simples (camisa social gasta, calça) modelada/marketplace.
5. Opcional pago: **Character Creator 5 + Headshot** ou um scan 3D contratado,
   se você quiser subir o realismo além do que o Creator entrega.

---

## 4. Danos progressivos no rosto — a parte mais delicada

Este é o diferencial do produto e onde a maioria dos projetos falha. O erro
clássico é aplicar o dano como **efeito de partícula** ou **decalque preso na
tela** — some quando a animação acaba, ou "flutua" quando o rosto se deforma.

### 4.1 Princípio: dano é **estado**, não efeito

Separo três camadas independentes, todas persistentes até o fim da rodada:

**Camada A — Tinta de material (tint/param).** Para vermelhidão, hematoma
difuso, palidez, suor, inchaço leve.
Implementação: material de pele customizado, derivado do MetaHuman, com máscaras
de região em UV space e escalares dirigidos por um *Material Parameter
Collection*. Custo: ~zero. Não deforma, não some, funciona em todos os LODs.

**Camada B — Malha de dano (skin layer).** Para olho roxo com contorno, curativo,
marca de impacto definida, lábio partido.
Implementação: **duplicata da malha da cabeça**, deslocada ~0,05 mm, *skinnada no
mesmo esqueleto e nos mesmos morph targets*. Isso é o que resolve o problema de
"o decalque nada quando o rosto se mexe". Custo: 1 draw call extra na cabeça.

**Camada C — Volumes e acessórios.** Inchaço de bochecha, curativo, dente
cenográfico.
- Inchaço: malha local + normal map, **não** morph target customizado no início
  (mexer em morphs do MetaHuman head é frágil e arrisca quebrar o RigLogic).
- Curativo: skeletal mesh preso ao osso da cabeça.
- Dente: máscara no material dos dentes + micro-malha lascada.

**Olhos:** param no material (veias/esclera avermelhada) + máscara de pálpebra
para o olho roxo. O olho do MetaHuman já tem shader de íris/córnea com parâmetros
expostos.

### 4.2 Modelo de estado

Um `struct` por região, com valores contínuos 0..1 e flags discretas:

```
faceDamage, leftEyeDamage, rightEyeDamage, mouthDamage,
cheekSwelling, foreheadDamage, jawDamage, noseDamage,
bodyDamage, balance, stunLevel, painLevel,
bBlackEye, bBandage, bToothChip, bLipSplit
```

Regra de aplicação por impacto:

1. O hit gera `hitLocation` + `hitNormal` + `intensity` + `type`.
2. Um **mapa de regiões** (volumes em volta de landmarks faciais, mais o osso
   atingido) converte isso em `(region, weight)`.
3. Aplica dano com **falloff** para regiões vizinhas (bater na bochecha afeta
   olho e mandíbula em 30–40%).
4. Aplica `stunLevel` e `balance` no controlador de física.
5. Recalcula o material e a malha de dano. **Nada é resetado.**

### 4.3 Persistência

Estado serializado em JSON por rodada, além de snapshot em memória. Só o
`MatchManager` no início de uma **nova rodada** chama `ResetDamage()`. Um golpe
nunca reseta nada — isso está no requisito 19.4 e vai ser testado explicitamente.

### 4.4 Direção de arte do dano

Seguindo sua escolha "danos visuais moderados": sem sangue, sem ferida aberta,
sem mutilação. O vocabulário é **cartoon-real**: vermelhidão, hematoma roxo,
inchaço, curativo, gelo, dente lascado, olho que não abre. Isso é mais engraçado
e mais seguro para a plataforma do que gore — e é o que sustenta a piada.

---

## 5. Física e ragdoll

### 5.1 O erro que mata a sensação: ragdoll total

Se a cada soco o personagem vira ragdoll completo, ele parece um boneco de pano
— e você já disse que não quer boneco. A solução é **ragdoll ativo com tônus
muscular variável**.

### 5.2 Arquitetura de reação em três estágios

**Estágio 1 — Hit reaction animada (leve).**
Montagem aditiva de reação (cabeça gira, tronco cede, pé recua) + *hitstop* de
2–4 frames + impulse físico pequeno. É o que dá "peso" ao golpe. 80% dos golpes
morrem aqui.

**Estágio 2 — Ragdoll parcial (médio).**
`PhysicalAnimationComponent` com força/damping por osso: tronco e braços perdem
tônus, pernas ainda tentam se sustentar. Resultado: o **cambaleio bêbado**, que é
exatamente o humor que você pediu. Duração 0,8–2,0 s.

**Estágio 3 — Ragdoll completo + queda (pesado/especial).**
Blend total para simulação, colapso, e depois **get-up**: animação de levantar
com IK de mão/pé + *ground trace*, sincronizada com a volta do capsule. Fases:
apoiar as mãos → ajoelhar → subir torto → recuperar postura.

Técnica de blend: curva de `PhysicsBlendWeight` dirigida por montagem
(`SetAllBodiesBelowPhysicsBlendWeight`), com CCD no pelvis e na cabeça.

### 5.3 Prevenção de artefatos (o que quebra ragdoll na prática)

| Problema | Solução |
|---|---|
| Atravessar o chão | CCD + shapes do PhysicsAsset calibradas + clamp de penetração no recover |
| Auto-interpenetração | Auto-colisão desligada entre ossos adjacentes; ligada só em pares distantes |
| Jitter/explosão | Limite de massa, damping, `bUseAsyncScene`, solver iterations fixo |
| Popa na transição | Curva de blend de 0,15 s + cópia de pose antes de simular |
| Personagem "escorrega" | Foot IK + friction no capsule |

**Chaos** (nativo da UE5) cobre tudo isso. Não há dependência externa.

### 5.4 Quem dá o golpe — decisão de produção que economiza meses

Você não pediu um agressor jogável. A escolha mais forte é **câmera em primeira
pessoa do agressor**: luvas, um taco, um chinelo entram em quadro vindos da
câmera. Isso:

- elimina a necessidade de um segundo personagem (‑1 MetaHuman, ‑1 rig, ‑1 anim BP);
- aumenta a imersão ("**eu** bati nele");
- deixa a câmera livre para os golpes cinematográficos;
- é o padrão de sucesso em conteúdo vertical de live.

Alternativa: silhueta do agressor no contraluz. Mais barato ainda.

---

## 6. Integração com TikTok LIVE

### 6.1 Realidade do ecossistema (sem inventar)

**Não existe API pública oficial da TikTok para ler presentes e comentários de
LIVE em tempo real.** Isso é fato, não limitação nossa.

O que existe:

| Caminho | Como funciona | Custo | Confiabilidade |
|---|---|---|---|
| **TikTokLive** (Python, `isaackogan/TikTokLive`) | Reverse-engineering do websocket do Webcast | Grátis | Quebra quando o TikTok muda assinatura; autor diz explicitamente "não é production-ready" |
| **tiktok-live-connector** (Node/TS, projeto irmão) | Mesmo princípio, em TS | Grátis | Mesma característica |
| **EulerStream** | WebSocket gerenciado; a Euler carrega a manutenção da assinatura | Tier grátis (2.500 req/dia, 25 sockets) + US$ 50/mês pro | Alta, é o que o próprio autor do TikTokLive recomenda para produção |
| API oficial TikTok | **Não expõe gifts/chat de LIVE** | — | — |

**Riscos que você precisa conhecer:**

1. O acesso por reverse-engineering viola os Termos do TikTok. Risco de bloqueio
   de IP/conta. É por isso que **o adaptador é isolado em processo separado**.
2. As libs quebram sem aviso. Precisa de manutenção contínua.
3. Em algumas regiões o Webcast exige sessão autenticada.
4. Presentes e catálogo variam por região (o Brasil tem gifting habilitado).

Vou implementar **dois adaptadores** desde o início: `TikTokLiveAdapter`
(gratuito, para desenvolvimento) e `EulerStreamAdapter` (interface pronta, chave
opcional). Trocar é uma linha de configuração.

### 6.2 Arquitetura — o jogo não conhece TikTok

```
TikTok LIVE
    │  (websocket, protocolo instável)
    ▼
┌───────────────────────────────────────────────┐
│  BRIDGE  (Node 24 / TypeScript, processo 1)   │
│                                               │
│  connectors/  → adapter escolhido por config  │
│  normalize/   → LiveEvent canônico            │
│  policy/      → dedupe, rate-limit, spam      │
│  store/       → SQLite/JSONL (log bruto)      │
│  adminApi/    → REST + WS para o painel       │
└───────────────────────────────────────────────┘
    │  WebSocket local (127.0.0.1, JSON)
    ▼
┌───────────────────────────────────────────────┐
│  UNREAL  (processo 2)                         │
│  receiver thread → TQueue (thread-safe)       │
│  → EventDirector (game thread, budget/tick)   │
│  → MatchManager → Combat → Damage/Physics     │
│  → UI / Audio / VFX                           │
└───────────────────────────────────────────────┘
    ▲
    │  REST + WS
┌───┴───────────────────────────────────────────┐
│  PAINEL ADMIN (React, navegador em localhost) │
└───────────────────────────────────────────────┘
```

**Contrato canônico** (`LiveEvent`):

```ts
type LiveEvent = {
  id: string            // idempotência
  kind: 'gift' | 'comment' | 'like' | 'follow' | 'join'
  ts: number
  user: { id: string, uniqueId: string, nickname: string, avatar?: string }
  gift?: { id: string, name: string, diamondCount: number, count: number,
           comboId?: string, comboEnd: boolean, streakable: boolean }
  comment?: { text: string }
  raw?: unknown         // preservado para debug
}
```

O jogo **só** entende esse contrato. Nenhum `#include` ou import de TikTok em
qualquer lugar do projeto UE. Isso é o requisito 8 na prática.

### 6.3 Semântica de combo (detalhe que quase todo mundo erra)

Presentes com *streak* disparam **vários** eventos, com `repeat_count`
incrementando, e o último vem com `repeat_end == 1`. Se você reagir a cada
evento, o personagem apanha 30 vezes por um combo de 5 rosas.

Regra: acumular por `comboId`, e **contabilizar** o total mesmo reagindo só no
fim. É exatamente o requisito 10 ("não descartar presentes silenciosamente,
preservar quantidades").

### 6.4 Fila, prioridade e o "diretor"

O `EventDirector` (C++, game thread, orçamento por tick) transforma eventos em
uma **linha do tempo de golpes**:

- cada golpe ocupa um slot (~0,7–1,2 s conforme tipo);
- fila com prioridade por intensidade, mas **com anti-inanição** (nada espera
  mais de N slots);
- agrupamento por `(usuário, presente)` dentro de janela de 1,5 s;
- no máximo 1 golpe + 1 reação secundária simultâneos (limite de animações);
- cooldown por usuário e cap por usuário (o excedente vira combo, não lixo);
- **modo hype:** se a fila passa de X eventos, um golpe especial consume o lote,
  com o contador da UI mostrando o total real.

Importante: a **chegada** de um presente e a **execução** de um golpe são
desacopladas. Atraso é aceitável; perda não é.

### 6.5 Latência esperada

| Etapa | Latência |
|---|---|
| TikTok → conector | 200–1500 ms (varia com a plataforma) |
| Bridge → UE (loopback) | < 2 ms |
| Entrada na fila → início do golpe | 0–1 slot (até ~1,2 s em pico) |

Não existe promessa de tempo real absoluto — a plataforma não permite. O design
absorve isso.

---

## 7. Performance na RTX 4060 — plano concreto

| Área | Decisão |
|---|---|
| Resolução | 1080×1920, 100%; TSR 67% se precisar de folga |
| FPS | `t.MaxFPS 30` fixo, VSync off. Frame time alvo ≤ 33 ms. |
| Iluminação | 1 directional estacionária com sombra + 2–3 rect lights sem sombra, tudo baked |
| GI | Lightmaps estáticos. Sem Lumen. |
| Reflexos | Sem SSR. Cubemap estático. |
| Ray tracing | **Desligado** |
| Cabelo | Groom LOD de produção. Sem sombra de fio. `r.HairStrands` afinado. |
| Texturas | Pool 5 GB, streaming agressivo para o cenário, 4K só na cabeça |
| Nanite | Apenas cenário. Personagem com LODs. |
| Shaders | PSO precache + cena de aquecimento no boot (esconde o stutter) |
| Assets | Todos pré-carregados no início da partida. Zero load em live. |
| Build | Standalone empacotado (`-game`), não editor |
| CPU | Eventos recebidos em thread dedicada; game thread nunca espera rede |
| Memória | Objetos de VFX em pool; sem alocação por golpe |
| Captura | OBS Game Capture (não captura de tela), encoder NVENC |

**Meta de aceitação:** 30 FPS estáveis por 20 minutos contínuos, com 100 eventos
injetados, VRAM < 7,2 GB, zero hitch > 50 ms.

---

## 8. Missão de arte e assets

### 8.1 O problema mais subestimado: animações de reação a golpe

Não existe biblioteca gratuita boa de "homem de 78 anos levando taco e
cambaleando". Mixamo tem reações genéricas de dano, de qualidade mediana e
retarget ruim para idoso.

Três caminhos, em ordem de custo-benefício:

1. **Mocap pack de hit reaction/fight** (Marketplace, US$ 30–80) + retarget e
   ajuste manual. Melhor custo/benefício.
2. **Mocap próprio** — um ator + traje/pinças. Precisa de espaço e tempo, mas
   resolve o "único" do produto.
3. **Animação manual keyframe** — viável para o ciclo de get-up, caro para tudo.

Recomendo (1) no início e (2) quando o produto já faturar.

### 8.2 Voz

Reações vocais de dor/tontura de um senhor: precisa de **gravação real ou
biblioteca licenciada**. Opções:

- ator de voz local, 1 sessão (~R$ 300–800): melhor resultado e autenticidade;
- biblioteca (US$ 50–150);
- **não** clonar voz de pessoa real — mesmo problema do rosto.

### 8.3 Audio design

Metasounds com camadas: impacto corporal + roupa + ambiente, variando pitch e
seleção entre 6–8 variantes por tipo. Reverb de estúdio. Sem repetição audível.

### 8.4 Cenário

Estúdio fechado: piso de concreto queimado, fundo com painéis acústicos e luzes
de LED em contraluz, alguns props (banco, caixote). Materiais PBR de marketplace
gratuito (Quixel/Fab free tier). Custo ~zero, resultado cinematográfico.

---

## 9. Interface

- UMG com canvas fixo 1080×1920 e *scale box* para adaptação.
- Elementos conforme §11 do briefing, com hierarquia: personagem domina o quadro;
  HUD nas bordas.
- Tipografia gratuita (Google Fonts / Fontshare) com licença comercial.
- Micro-animações via materiais e *Sequencer* em widgets, não GIFs.
- Regra de contenção: no máximo 5 elementos informativos simultâneos.
- "FICTIONAL CHARACTER" / "PERSONAGEM FICTÍCIO" permanente e discreto — proteção
  jurídica e de plataforma.

---

## 10. Painel administrativo

Aplicação web local (React + Vite) servida pelo bridge em
`http://localhost:5174`. Nada de instalar app.

Capacidades (requisito 15): iniciar/encerrar rodada, resetar personagem, modo
simulação, disparar golpes manualmente, editar mapeamento de presentes
(tipo/intensidade/animação/som/VFX/multiplicador), ajustes gráficos, log de
eventos, log de erros de conexão, toggles de efeito, estatísticas.

Configuração vive em **JSON fora do código** (`config/gifts.json`,
`config/attacks.json`, `config/scene.json`), versionada no git. O jogo observa o
arquivo e recarrega a quente.

---

## 11. Cronograma realista

Premissas: 1 desenvolvedor, dedicação full-time, com assistência de IA.
Multiplique por ~1,8 se for em paralelo a outro trabalho.

| Fase | Entrega | Tempo | Gate de aceite |
|---|---|---|---|
| 0. Setup | Engine, VS, projeto, repositório, CI local | 3–5 dias | Compila e roda vazio |
| 1. Planejamento fino | Esquemas de config, contratos de evento, esqueleto de módulos | 3–4 dias | Revisão de arquitetura aprovada |
| 2. Protótipo visual | Estúdio, MetaHuman original, câmera, luz, look-dev | **2–3 semanas** | **Gate A:** screenshot 1080×1920 a 30 FPS aprovado por você |
| 3. Mecânica | Golpes, hit reactions, ragdoll ativo, get-up | **4–6 semanas** | **Gate B:** um soco e uma queda "sentem" certo |
| 4. Dano progressivo | Camadas A/B/C, mapa de regiões, persistência | **3–4 semanas** | **Gate C:** 20 golpes, dano acumula e não reseta |
| 5. Integração | Bridge, adaptadores, fila, combos, simulação | 2–3 semanas | **Gate D:** 100 eventos em 10 s sem travar |
| 6. UI + Painel | HUD, ranking, admin, estatísticas | 3 semanas | Fluxo completo de rodada |
| 7. Otimização e live | VRAM, PSO, teste 20 min, OBS | 2 semanas | **Gate E:** live de teste estável |
| **Total** | | **~4–5 meses** | |

**Marcos intermediários honestos:**

- **8–10 semanas:** *vertical slice* convincente (1 personagem, 3 golpes, dano,
  HUD básico) — dá para mostrar e validar.
- **4–5 meses:** produto com a polidez de "jogo comercial" que você exigiu.
- A diferença entre os dois é quase toda **polimento de animação e feel** — a
  parte que não tem atalho.

---

## 12. Custos

| Item | Custo |
|---|---|
| Unreal Engine 5 | R$ 0 (5% de royalty só acima de US$ 1M/receita) |
| MetaHuman Creator + Animator | R$ 0 |
| Chaos, Nanite, UMG, Metasounds | R$ 0 |
| Node.js, React, SQLite | R$ 0 |
| OBS Studio | R$ 0 |
| TikTokLive / tiktok-live-connector | R$ 0 (AGPL — atenção se for fechar o código) |
| EulerStream | US$ 0 (tier) a US$ 50/mês |
| Mocap packs | US$ 30–80 |
| SFX | US$ 50–150 |
| VO | R$ 300–800 |
| **Total necessário** | **R$ 0** |
| **Total recomendado (qualidade)** | **~US$ 200–600** |

**Atenção de licença:** TikTokLive é AGPL-3.0. Rodar como processo separado,
comunicando por rede, evita contaminar o código do jogo. Isso reforça a escolha
de arquitetura.

---

## 13. Riscos

| Risco | Prob. | Impacto | Mitigação |
|---|---|---|---|
| Disco insuficiente | **Alta** | **Bloqueia tudo** | Liberar 60–100 GB antes da Fase 2 |
| Conector TikTok quebrar | Alta | Alto | Adaptadores plugáveis + EulerStream como plano B |
| Banimento da conta por conteúdo | Média | Crítico | Personagem original, violência não gráfica, rótulo de ficção, sem gore |
| VRAM estourar no close-up | Média | Alto | Sem Lumen, groom enxuto, build empacotado, orçamento medido |
| Animação de hit reaction fraca | **Alta** | Alto | Mocap pack + polimento; é o maior risco de "feel" |
| Ragdoll instável | Média | Médio | 3 estágios de reação, testes automatizados de queda |
| Latência de eventos | Certa | Baixo | Fila desacoplada, UI otimista |
| Escopo crescer | Alta | Alto | Gates obrigatórios; nada avança sem aprovação |

---

## 14. Plano de execução (o que acontece depois da sua aprovação)

**Passo 1 — Decisões suas (deste documento):**
1. Personagem original em vez de likeness real — confirma?
2. Liberar espaço em disco (ou autorizar instalação no D: com limpeza).
3. Aprovar "agressor em primeira pessoa" em vez de segundo personagem.
4. Definir orçamento de assets (R$ 0 ou ~US$ 300).

**Passo 2 — Infra:** instalar UE 5.6 + MetaHuman Core Data + VS2022,
criar projeto, estrutura de módulos, repositório, README de setup.

**Passo 3 — Fase 2 e Gate A:** cenário + personagem + câmera + luz, entregue
como screenshot 1080×1920 e clipe de 10 s a 30 FPS. **Nada de mecânica antes de
você aprovar o visual.**

Só depois disso começam golpes, física e dano.

---

## 15. Decisões aprovadas

Registradas após a apresentação da análise:

| # | Decisão | Consequência |
|---|---|---|
| 1 | **Personagem original** confirmado | MetaHuman desenhado do zero, identidade própria. Fotos de `referencia_visual/` descartadas. |
| 2 | **Instalação no D:** | Engine, VS2022 e projeto em `D:\`. Ver `docs/SETUP_AMBIENTE.md` para a matemática de disco. |
| 3 | **Agressor em primeira pessoa** | Um único MetaHuman no projeto. Câmera subjetiva com luvas/taco/chinelo entrando em quadro. |
| 4 | **Orçamento R$ 0 por enquanto** | Só assets gratuitos (Mixamo, Fab free tier, Freesound CC0). Mocap pago e VO ficam para quando houver receita — **isso limita o "feel" das reações e está assumido como risco**. |

### 15.1 Impacto do orçamento zero

Sem mocap pago, a Fase 3 passa a depender de:

- animações do **Mixamo** (gratuitas, mas de qualidade mediana e retarget ruim para idoso);
- pacotes **gratuitos da Fab** (antigo Marketplace), sazonais;
- **animação manual em keyframe** para o ciclo de get-up e as reações principais —
  mais lento, porém sob controle total;
- **procedural**: o ragdoll ativo e as reações de física compensam boa parte da
  falta de mocap, porque o movimento passa a ser gerado pelo Chaos, não pela
  animação. É aqui que a arquitetura escolhida ajuda.

Expectativa honesta: o resultado será convincente, mas o polimento de animação
que separa "jogo indie bom" de "jogo comercial" vai exigir mocap depois.

### 15.2 O que continua valendo do briefing

Todos os requisitos técnicos de 5 a 17 permanecem integralmente: golpes com
colisão e física, dano progressivo persistente, ragdoll, integração com fila e
combos, HUD vertical, painel admin, ciclo de rodada, áudio sincronizado.

---

## 16. Resumo das divergências conscientes do briefing

Para ser explícito, já que o requisito 19.8 pede transparência:

1. **Personagem:** não será likeness de pessoa real. Original, com a mesma
   qualidade técnica. (§3)
2. **Lumen/ray tracing:** desligados por orçamento de VRAM. Iluminação baked
   entrega resultado igual ou melhor para estúdio fechado. (§1.2, §7)
3. **Tempo:** 4–5 meses, não semanas. É o prazo real para o padrão pedido. (§11)
4. **Integração TikTok:** é não oficial e pode quebrar. Não vou fingir que
   existe uma API estável. (§6.1)
