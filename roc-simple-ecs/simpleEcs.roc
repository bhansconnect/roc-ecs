app "roc-simple-ecs"
    packages { pf: "." }
    imports []
    provides [ dummy ] to pf

# This is just a dummy.
# Everything is defined in Package-Config because we can't require multiple functions.

dummy = {}