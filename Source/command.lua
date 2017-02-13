local _M = {}

-- convert between the letters and command names
_M.name = {
    T = "squad_to_abs",
    t = "squad_to_rel",
    R = "rquad_to_abs",
    r = "rquad_to_rel",
    A = "svgarc_to_abs",
    a = "svgarc_to_rel",
    C = "cubic_to_abs",
    c = "cubic_to_rel",
    H = "hline_to_abs",
    h = "hline_to_rel",
    L = "line_to_abs",
    l = "line_to_rel",
    M = "move_to_abs",
    m = "move_to_rel",
    Q = "quad_to_abs",
    q = "quad_to_rel",
    S = "scubic_to_abs",
    s = "scubic_to_rel",
    V = "vline_to_abs",
    v = "vline_to_rel",
    Z = "close_path", -- doesn't really matter if relative or not
    z = "close_path",
}

-- gives the number of arguments each of the commands take
_M.nargs = {
    T = 2,
    t = 2,
    R = 5,
    r = 5,
    A = 7,
    a = 7,
    C = 6,
    c = 6,
    H = 1,
    h = 1,
    L = 2,
    l = 2,
    M = 2,
    m = 2,
    Q = 4,
    q = 4,
    S = 4,
    s = 4,
    V = 1,
    v = 1,
    Z = 0,
    z = 0
}

-- convert between the names and letters
_M.letter = { }

-- make sure the names are defined to the same value as the letters
for el, n in pairs(_M.name) do
    -- export commands into module table
    _M[el] = n
    _M[n] = n
    -- generate the shortname translate table from long
    _M.letter[n] = el
    -- copy nargs from short to long
    _M.nargs[n] = _M.nargs[el]
end

return _M
