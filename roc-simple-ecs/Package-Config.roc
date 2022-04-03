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
stepForHost = \boxModel, currentFrame, _spawnRate, _particles ->
    model0 = Box.unbox boxModel
    model1 = deathSystem model0 currentFrame
    toDraw = graphicsSystem model1
    {model: Box.box model1, toDraw}

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
                            deadSigniture = Signiture.removeAlive  signiture
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
