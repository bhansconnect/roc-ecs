interface Program
    exposes [ Program ]
    imports []

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

Program model :
    {
        initFn: U32, I32 -> Box model,
        setMaxFn: Box model, I32 -> Box model,
        sizeFn: Box model -> { model: Box model, size: I32 },
        stepFn: Box model, I32, F32, I32 -> { model: Box model, toDraw: List ToDraw },
    }