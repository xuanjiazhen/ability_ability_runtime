class BaseContest{
    static { loadLibrary("napi_base_context.z");}
    stageMode : boolean = true;
}

class Contest{
    public nativeContext : long;
}
