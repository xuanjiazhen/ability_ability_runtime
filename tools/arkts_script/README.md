# arktsScript

`arktsScript` is a lightweight ArkTS script runner for executing compiled ArkCompiler bytecode (`.abc`) files.

## Usage

### Command template

```bash
arktsScript --abcPath="<abc path>" --functionName="<function name>" --arg1="<typedArg1>" --arg2="<typedArg2>" ...
```

### Parameters

| Parameter | Required | Description |
|------|------|------|
| `--abcPath` | Yes | Path to the compiled `.abc` file |
| `--functionName` | Yes | Global function name to invoke |
| `--argN` | No | Typed function argument, sequential from `--arg1` |
| `-h`, `--help` | No | Print help and exit |

### Argument types

| Type | Example |
|------|------|
| `string` | `str:hello` |
| `number` | `num:123` |
| `boolean` | `bool:true` |
| `array` | `array:[1,2,3]` |
| `object` | `jsonString:{"key":"value"}` |

## Examples

```bash
arktsScript --abcPath="/data/scripts/test.abc" --functionName="main"
arktsScript --abcPath="/data/scripts/test.abc" --functionName="add" --arg1="num:10" --arg2="num:20"
arktsScript --abcPath="/data/scripts/test.abc" --functionName="configure" --arg1="str:prod" --arg2="num:8080" --arg3="bool:true"
arktsScript --abcPath="/data/scripts/test.abc" --functionName="update" --arg1="jsonString:{\"name\":\"test\",\"value\":42}"
arktsScript --help
```

## CLI manager params

When invoking `arktsScript` through the JS CLI manager, use:

```ts
{
  abcPath: "/data/scripts/test.abc",
  functionName: "main",
  arg1: "hello",
  arg2: 42,
  arg3: true,
  arg4: [1, 2],
  arg5: { a: 1 }
}
```

The CLI manager serializes these values into explicit typed CLI arguments:

```bash
--abcPath="/data/scripts/test.abc" --functionName="main" --arg1="str:hello" --arg2="num:42" --arg3="bool:true"
```
