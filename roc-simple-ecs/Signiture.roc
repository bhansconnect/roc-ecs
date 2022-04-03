interface Signiture
    exposes [
        Signiture,
        matches,
        empty,
        feelsGravity,
        setDeathTime,
        setFade,
        setExplode,
        setGraphic,
        setPosition,
        setVelocity,
        setAlive,
        removeAlive,
        isAlive,
    ]
    imports []


Signiture := U8

empty : Signiture
empty = $Signiture 0

matches : Signiture, Signiture -> Bool
matches = \$Signiture main, $Signiture other ->
    Num.bitwiseAnd main other == other

feelsGravity : Signiture -> Signiture
feelsGravity = \$Signiture sig -> $Signiture (Num.bitwiseOr sig 0b0000_0001)

setDeathTime : Signiture -> Signiture
setDeathTime = \$Signiture sig -> $Signiture (Num.bitwiseOr sig 0b0000_0010)

setFade : Signiture -> Signiture
setFade = \$Signiture sig -> $Signiture (Num.bitwiseOr sig 0b0000_0100)

setExplode : Signiture -> Signiture
setExplode = \$Signiture sig -> $Signiture (Num.bitwiseOr sig 0b0000_1000)

setGraphic : Signiture -> Signiture
setGraphic = \$Signiture sig -> $Signiture (Num.bitwiseOr sig 0b0001_0000)

setPosition : Signiture -> Signiture
setPosition = \$Signiture sig -> $Signiture (Num.bitwiseOr sig 0b0010_0000)

setVelocity : Signiture -> Signiture
setVelocity = \$Signiture sig -> $Signiture (Num.bitwiseOr sig 0b0100_0000)

setAlive : Signiture -> Signiture
setAlive = \$Signiture sig -> $Signiture (Num.bitwiseOr sig 0b1000_0000)

removeAlive : Signiture -> Signiture
removeAlive = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b0111_1111)

isAlive : Signiture -> Bool
isAlive = \sig -> matches sig (empty |> setAlive)
# Swap impl once bug fixed
# isAlive = \$Signiture sig -> sig >= 0b1000_0000
