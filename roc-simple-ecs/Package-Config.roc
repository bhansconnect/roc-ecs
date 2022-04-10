platform "roc-ecs-test"
    requires { Model } { main : Effect {} }
    exposes []
    packages {}
    imports [ Program.{ ToDraw } ]
    provides [ mainForHost ]

mainForHost :
    {
        initFn: (U32, I32 -> Box Model) as InitFn,
        setMaxFn: (Box Model, I32 -> Box Model) as SetMaxFn,
        sizeFn: (Box Model -> { model: Box Model, size: I32 }) as SizeFn,
        stepFn: (Box Model, I32, F32, I32 -> { model: Box Model, toDraw: List ToDraw }) as StepFn,
    }
mainForHost = main