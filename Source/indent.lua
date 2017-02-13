local _M = {}

function _M.indent(n, s)
    local indent = { n = n, s = s, v = s:rep(n) }

    function indent.write(self, file)
        file:write("\n", self.v)
    end

    function indent.inc(self)
        self.n = self.n + 1
        self.v = self.s:rep(self.n)
    end

    function indent.write_inc(self, file)
        self:write(file)
        self:inc()
    end

    function indent.dec(self)
        self.n = self.n - 1
        self.v = self.s:rep(self.n)
    end

    function indent.dec_write(self, file)
        self:dec()
        self:write(file)
    end

    return indent
end

return _M
