platform "roc-ecs-test"
    requires {} { dummy: {} }
    exposes []
    packages {}
    imports [ Random ]
    provides [ dummyForHost, initForHost, stepForHost, setMaxForHost, sizeForHost ]

dummyForHost = dummy

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

Signiture := U8

Entity : {
    id: I32,
    signiture: Signiture,
}

genEntities : Nat -> List Entity
genEntities = \count ->
    base = List.repeat {id: 0, signiture: $Signiture 0} count
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
}

initForHost : U32, I32 -> Box Model
initForHost = \seed, max ->
    maxNat = Num.toNat max
    entities = genEntities maxNat
    Box.box {
        rng: Random.seed32 seed,
        max,
        entities,
        size: 0,
        nextSize: 0,
    }

setMaxForHost : Box Model, I32 -> Box Model
setMaxForHost = \boxModel, max ->
    model = Box.unbox boxModel
    Box.box {model & max, size: 0, nextSize: 0}

sizeForHost : Box Model -> I32
sizeForHost = \boxModel ->
    model = Box.unbox boxModel
    model.size

stepForHost : Box Model, I32, F32, I32 -> { model: Box Model, toDraw: List ToDraw }
stepForHost = \boxModel, _currentFrame, _spawnRate, _particles ->
    {model: boxModel, toDraw: [ { color: { aB: 255, bG: 0, cR: 0, dA: 255 }, radius: 0.04, x: 0.5, y: 0.5 } ]}
