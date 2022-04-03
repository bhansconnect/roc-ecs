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

Model : {
    rng: Random.State U32,
    max: I32,
    size: I32,
}

initForHost : U32, I32 -> Box Model
initForHost = \seed, max ->
    Box.box {
        rng: Random.seed32 seed,
        max,
        size: 0,
    }

setMaxForHost : Box Model, I32 -> Box Model
setMaxForHost = \boxModel, max ->
    model = Box.unbox boxModel
    Box.box {model & max, size: 0}

sizeForHost : Box Model -> I32
sizeForHost = \boxModel ->
    model = Box.unbox boxModel
    model.size

stepForHost : Box Model, I32, F32, I32 -> { model: Box Model, toDraw: List ToDraw }
stepForHost = \boxModel, _currentFrame, _spawnRate, _particles ->
    {model: boxModel, toDraw: [ { color: { aB: 255, bG: 0, cR: 0, dA: 255 }, radius: 0.04, x: 0.5, y: 0.5 } ]}
