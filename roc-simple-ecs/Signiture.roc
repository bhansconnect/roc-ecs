interface Signiture
    exposes [ Signiture, matches, empty ]
    imports []


Signiture := U8

empty : Signiture
empty = $Signiture 0

matches : Signiture, Signiture -> Bool
matches = \$Signiture main, $Signiture other ->
    Num.bitwiseAnd main other == other
