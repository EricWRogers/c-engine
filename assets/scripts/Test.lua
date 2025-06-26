Test = {
    x = 5
}

function Test.Start()
    Log("Start " .. x)
    x = x + 1;
end

function Test.Update()
    Log("Update")
    x = x + 1;
end

function Test.OnDestroy()
    Log("Destroyed")
end
