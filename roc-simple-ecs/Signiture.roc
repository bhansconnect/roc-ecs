interface Signiture
    exposes [
        Signiture,
        matches,
        empty,
        feelsGravity,
        hasDeathTime,
        hasFade,
        hasExplode,
        hasGraphic,
        hasPosition,
        hasVelocity,
        setAlive,
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
feelsGravity = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b0000_0001)

hasDeathTime : Signiture -> Signiture
hasDeathTime = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b0000_0010)

hasFade : Signiture -> Signiture
hasFade = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b0000_0100)

hasExplode : Signiture -> Signiture
hasExplode = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b0000_1000)

hasGraphic : Signiture -> Signiture
hasGraphic = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b0001_0000)

hasPosition : Signiture -> Signiture
hasPosition = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b0010_0000)

hasVelocity : Signiture -> Signiture
hasVelocity = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b0100_0000)

setAlive : Signiture -> Signiture
setAlive = \$Signiture sig -> $Signiture (Num.bitwiseAnd sig 0b1000_0000)

isAlive : Signiture -> Bool
isAlive = \$Signiture sig -> sig >= 0b1000_0000
