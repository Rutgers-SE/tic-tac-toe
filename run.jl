if length(ARGS) < 1
    exit()
end
values = split(strip(readlines(pipeline(`ifconfig wlp3s0`, `grep "inet "`))[1]), " ")
i=indexin(["inet"], values)
s = values[i+1]
println(s[1]*":$(ARGS[1])")
