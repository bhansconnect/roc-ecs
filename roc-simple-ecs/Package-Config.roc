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

initForHost : U32 -> Model
initForHost = \seed ->
    {
        rng: Random.seed32 seed,
    }


stepForHost : Model -> { state: U32, value: U32 }
stepForHost = \model ->
    dist = (Random.u32 0 128)
    next = dist model.rng
    { value: next.value, state: Random.extract32 next.state }
