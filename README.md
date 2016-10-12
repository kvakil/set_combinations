itertools.combinations variant which generates sets.

Usage:

    >>> from set_combinations import set_combinations
    >>> list(set_combinations(range(5), 2))
    [set([0, 1]), set([0, 2]), set([0, 3]), set([0, 4]), set([1, 2]),
     set([1, 3]), set([1, 4]), set([2, 3]), set([2, 4]), set([3, 4])]

Remember that sets, with all their properties, are returned:

    >>> list(set_combinations([0, 0, 1, 2], 2))
    [set([0]), set([0, 1]), set([0, 2]), set([0, 1]), set([0, 2]), set([1, 2])]

For the expected results, you can pass in a set yourself:

    >>> list(set_combinations({0, 0, 1, 2}, 2))
    [set([0, 1]), set([0, 2]), set([1, 2])]

Using set_combinations() is about 40% faster than something like:

    >>> for s in itertools.combinations(range(20), 10):
    ...    set(s)
