platform "roc-ecs-test"
    requires {} { dummy: {} }
    exposes []
    packages {}
    imports [ Random, Signiture.{ Signiture } ]
    provides [ dummyForHost, initForHost, stepForHost, setMaxForHost, sizeForHost ]

dummyForHost = dummy

twoPi: F32
twoPi = 2.0 * Num.acos -1

# Names are weird here cause we need to match SDL ordering.
Color : {
    aB: U8,
    bG: U8,
    cR: U8,
    dA: U8,
}

ToDraw : {
    color: Color,
    radius: F32,
    x: F32,
    y: F32,
}

Entity : {
    id: I32,
    signiture: Signiture,
}

CompDeathTime : {
    deadFrame: I32,
}

CompFade : {
    rRate: U8,
    rMin: U8,
    gRate: U8,
    gMin: U8,
    bRate: U8,
    bMin: U8,
    aRate: U8,
    aMin: U8,
}

CompExplode : {
    numParticles: I32
}

CompGraphic : {
    color: Color,
    radius: F32,
}

CompPosition : {
    x: F32,
    y: F32,
}

CompVelocity : {
    dx: F32,
    dy: F32,
}

genEntities : Nat -> List Entity
genEntities = \count ->
    base = List.repeat {id: 0, signiture: Signiture.empty} count
    genEntitiesHelper base 0

genEntitiesHelper : List Entity, Nat -> List Entity
genEntitiesHelper = \entities, index ->
    when List.get entities index is
        Ok entity ->
            nextEntities = List.set entities index {entity & id: Num.toI32 index}
            genEntitiesHelper nextEntities (index + 1)
        Err OutOfBounds ->
            entities

Model : {
    rng: Random.State U32,
    max: I32,
    size: I32,
    nextSize: I32,
    entities: List Entity,
    deathTimes: List CompDeathTime,
    fades: List CompFade,
    explodes: List CompExplode,
    graphics: List CompGraphic,
    positions: List CompPosition,
    velocities: List CompVelocity,
}

initForHost : U32, I32 -> Box Model
initForHost = \seed, max ->
    maxNat = Num.toNat max
    Box.box {
        rng: Random.seed32 seed,
        max,
        size: 0,
        nextSize: 0,
        entities:  genEntities maxNat,
        deathTimes: List.repeat { deadFrame: 0 } maxNat,
        fades: List.repeat { rRate: 0, rMin: 0, gRate: 0, gMin: 0, bRate: 0, bMin: 0, aRate: 0, aMin: 0 } maxNat,
        explodes: List.repeat { numParticles: 0 } maxNat,
        graphics: List.repeat { color: { aB: 0, bG: 0, cR: 0, dA: 0 }, radius: 0.0 } maxNat,
        positions: List.repeat { x: 0.0, y: 0.0 } maxNat,
        velocities: List.repeat { dx: 0.0, dy: 0.0 } maxNat,
    }

setMaxForHost : Box Model, I32 -> Box Model
setMaxForHost = \boxModel, max ->
    maxNat = Num.toNat max
    model = Box.unbox boxModel
    Box.box {model &
        max,
        size: 0,
        nextSize: 0,
        entities: genEntities maxNat,
        deathTimes: List.repeat { deadFrame: 0 } maxNat,
        fades: List.repeat { rRate: 0, rMin: 0, gRate: 0, gMin: 0, bRate: 0, bMin: 0, aRate: 0, aMin: 0 } maxNat,
        explodes: List.repeat { numParticles: 0 } maxNat,
        graphics: List.repeat { color: { aB: 0, bG: 0, cR: 0, dA: 0 }, radius: 0.0 } maxNat,
        positions: List.repeat { x: 0.0, y: 0.0 } maxNat,
        velocities: List.repeat { dx: 0.0, dy: 0.0 } maxNat,
    }

sizeForHost : Box Model -> { model: Box Model, size: I32 }
sizeForHost = \boxModel ->
    model = Box.unbox boxModel
    { model: boxModel, size: model.size }

stepForHost : Box Model, I32, F32, I32 -> { model: Box Model, toDraw: List ToDraw }
stepForHost = \boxModel, currentFrame, spawnRate, particles ->
    model0 = Box.unbox boxModel
    model1 = deathSystem model0 currentFrame
    model2 = spawnSystem model1 currentFrame spawnRate particles
    toDraw = graphicsSystem model2
    {model: Box.box model2, toDraw}

addEntity : Model, Signiture -> Result { model: Model, id: I32 } [ OutOfSpace Model ]
addEntity = \model, signiture ->
    {nextSize, max, entities} = model
    if nextSize < max then
        nextSizeNat = Num.toNat nextSize
        when List.get entities nextSizeNat is
            Ok {id} ->
                nextModel = 
                    { model & entities: List.set entities nextSizeNat { id, signiture }, nextSize: nextSize + 1, size: nextSize + 1 }
                Ok { model: nextModel, id }
            Err OutOfBounds ->
                # This should be impossible.
                Ok { model, id: Num.minI32 - 1 }
    else
        Err (OutOfSpace model)

divByNonZeroF32 : F32, F32 -> F32
divByNonZeroF32 = \a, b ->
    when a / b is
        Ok v -> v
        _ -> 0 # Garbage value

# Num.round seems to be stuck using one type
numRoundI32 : F32 -> I32
numRoundI32 = \x -> Num.toI32 (Num.round x)

numRoundU32 : F32 -> U32
numRoundU32 = \x -> Num.toU32 (Num.round x)

spawnFirework : Model, I32, I32 -> Result Model [ OutOfSpace Model ]
spawnFirework = \model0, currentFrame, numParticles ->
    signiture =
        Signiture.empty
        |> Signiture.setAlive
        |> Signiture.setDeathTime
        |> Signiture.setExplode
        |> Signiture.setGraphic
        |> Signiture.setPosition
        |> Signiture.setVelocity
    when addEntity model0 signiture is
        Ok result ->
            model1 = result.model
            # All rand mapped so that 0.0 to 1.0 is 0 to 1,000,000
            riseSpeedRand = (Random.u32 10_000 25_000) model1.rng
            riseSpeed = divByNonZeroF32 (Num.toFloat riseSpeedRand.value) 1_000_000.0
            framesToCrossScreen = divByNonZeroF32 1.0 riseSpeed
            lifeMin = numRoundU32 (framesToCrossScreen * 0.6 * 1_000_000.0)
            lifeMax = numRoundU32 (framesToCrossScreen * 0.95 * 1_000_000.0)
            lifeInFramesRand = (Random.u32 lifeMin lifeMax) riseSpeedRand.state
            lifeInFrames = divByNonZeroF32 (Num.toFloat lifeInFramesRand.value) 1_000_000.0
            deadFrame = currentFrame + (numRoundI32 lifeInFrames)

            colorRand = (Random.u32 0 2) lifeInFramesRand.state
            color: Color
            color =
                when colorRand.value is
                    0 -> { aB: 255, bG: 0, cR: 0, dA: 255}
                    1 -> { aB: 0, bG: 255, cR: 0, dA: 255}
                    2 -> { aB: 0, bG: 0, cR: 255, dA: 255}
                    _ -> { aB: 0-1, bG: 0, cR: 0, dA: 255} # This should be impossible

            xRand = (Random.u32 50_000 950_000) colorRand.state
            x = divByNonZeroF32 (Num.toFloat xRand.value) 1_000_000.0
            id = Num.toNat result.id
            Ok {
                model1 &
                rng: xRand.state,
                deathTimes: List.set model1.deathTimes id { deadFrame },
                explodes: List.set model1.explodes id { numParticles },
                graphics: List.set model1.graphics id { color, radius: 0.02 },
                positions: List.set model1.positions id { x, y: 0.0 },
                velocities: List.set model1.velocities id { dx: 0, dy: riseSpeed },
            }
        Err (OutOfSpace model1) ->
            Err (OutOfSpace model1) 

spawnSystem : Model, I32, F32, I32 -> Model
spawnSystem = \model, currentFrame, spawnRate, numParticles ->
    guaranteedSpawns = Num.toI32 (Num.floor spawnRate)
    spawnSystemHelper model currentFrame spawnRate numParticles guaranteedSpawns 0

spawnSystemHelper : Model, I32, F32, I32, I32, I32 -> Model
spawnSystemHelper = \model0, currentFrame, spawnRate, numParticles, guaranteedSpawns, i ->
    if i < guaranteedSpawns then
        when spawnFirework model0 currentFrame numParticles is
            Ok model1 ->
                spawnSystemHelper model1 currentFrame spawnRate numParticles guaranteedSpawns (i+1)
            Err (OutOfSpace model1) ->
                model1
    else
        remainingSpawnRate = spawnRate - (Num.toFloat guaranteedSpawns)
        remainingSpawnRateU32 = numRoundU32 (remainingSpawnRate * 1_000_000.0)
        spawnRand = (Random.u32 0 1_000_000) model0.rng
        model1 = { model0 & rng: spawnRand.state }
        if spawnRand.value < remainingSpawnRateU32 then
            when spawnFirework model1 currentFrame numParticles is
                Ok model2 -> model2
                Err (OutOfSpace model2) -> model2
        else
            model1

deathSystem : Model, I32 -> Model
deathSystem = \model, currentFrame ->
    deathSystemHelper model currentFrame 0

deathSystemHelper : Model, I32, I32 -> Model
deathSystemHelper = \model, currentFrame, i ->
    if i < model.size then
        when List.get model.entities (Num.toNat i) is
            Ok { id, signiture } ->
                when List.get model.deathTimes (Num.toNat id) is
                    Ok { deadFrame } ->
                        # TODO: maybe make this branchless for performance
                        if currentFrame < deadFrame then
                            deathSystemHelper model currentFrame (i + 1)
                        else
                            deadSigniture = Signiture.removeAlive signiture
                            nextModel = { model & entities: List.set model.entities (Num.toNat i) { id, signiture: deadSigniture } }
                            deathSystemHelper nextModel currentFrame (i + 1)
                    Err OutOfBounds ->
                        # This should be impossible
                        deathSystemHelper model currentFrame (Num.minI32 - 1)
            Err OutOfBounds ->
                # This should be impossible
                deathSystemHelper model currentFrame (Num.minI32 - 1)
    else
        model

graphicsSystem : Model -> List ToDraw
graphicsSystem = \{size, entities, graphics, positions} ->
    # This really should be some for of reserve, but it doesn't exist yet.
    base = List.repeat { color: { aB: 0, bG: 0, cR: 0, dA: 0 }, radius: 0.0, x: 0.0, y: 0.0 } (Num.toNat size)
    toMatch = Signiture.empty |> Signiture.setAlive |> Signiture.setGraphic |> Signiture.setPosition
    out =
        # TODO: This should stop when we pass reach size.
        List.walk entities {index: Num.toNat 0, count: Num.toNat 0, toDraw: base} (\{index, count, toDraw}, {id, signiture} ->
            idNat = Num.toNat id
            if Signiture.matches signiture toMatch then
                when List.get graphics idNat is
                    Ok {color, radius} ->
                        when List.get positions idNat is
                            Ok {x, y} ->
                                nextToDraw = List.set toDraw count { color, radius, x, y }
                                { index: index + 1, count: count + 1, toDraw: nextToDraw }
                            Err OutOfBounds ->
                                # This should be impossible.
                                { index: 0 - 1, count, toDraw }
                    Err OutOfBounds ->
                        # This should be impossible.
                        { index: 0 - 1, count, toDraw }
            else
                { index: index + 1, count, toDraw }
        )
    # TODO: verify that this can modify in place and be fast.
    List.takeFirst out.toDraw out.count
