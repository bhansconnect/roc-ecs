platform "roc-ecs-test"
    requires {} { init : {} -> _ }
    exposes []
    packages {}
    imports []
    provides [ initForHost ]

initForHost : {} -> _
initForHost = \_ -> init {}
