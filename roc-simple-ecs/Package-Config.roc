platform "roc-ecs-test"
    requires {} { dummy: {} }
    exposes []
    packages {}
    imports [ Random ]
    provides [ dummyForHost, initForHost, stepForHost ]

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
}

initForHost : U32 -> Box Model
initForHost = \seed ->
    Box.box {
        rng: Random.seed32 seed,
    }


stepForHost : Box Model -> { model: Box Model, toDraw: List ToDraw }
stepForHost = \boxModel ->
    {model: boxModel, toDraw: [ { color: { aB: 255, bG: 0, cR: 0, dA: 255 }, radius: 0.04, x: 0.5, y: 0.5 } ]}
