platform "roc-ecs-test"
    requires {} { dummy: {} }
    exposes []
    packages {}
    imports [ Random ]
    provides [ dummyForHost, initForHost, stepForHost ]

dummyForHost = dummy

Model : {
    rng: Random.State U32,
}

initForHost : U32 -> Box Model
initForHost = \seed ->
    Box.box {
        rng: Random.seed32 seed,
    }


stepForHost : Box Model -> { model: Box Model, value: U32 }
stepForHost = \boxModel ->
    model = Box.unbox boxModel
    dist = (Random.u32 0 128)
    rand = dist model.rng
    nextModel = {model & rng: rand.state}
    nextBoxModel = Box.box nextModel
    {model: nextBoxModel, value: rand.value}
