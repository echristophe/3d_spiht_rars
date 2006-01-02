Compression and decompression ok
Performances are not that high due to a tree-crossing problem. Early output can be caused independantly on two branches of the tree.

Possible solutions:
- detect coefficient which caused an early output in a previous LIS element (this coefficient will be processed later anyway).
- change tree structure to become a one-parent relationship.


