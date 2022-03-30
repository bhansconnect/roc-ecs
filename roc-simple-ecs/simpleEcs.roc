app "roc-simple-ecs"
    packages { pf: "." }
    imports []
    provides [ init ] to pf

init : {} -> I64
init = \_ -> 12