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


stepForHost : Model -> U32
stepForHost = \{rng} ->
    dist = (Random.u32 0 128)
    next = dist rng
    next.value
