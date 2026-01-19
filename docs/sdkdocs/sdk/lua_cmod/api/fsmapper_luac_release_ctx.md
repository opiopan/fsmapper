# fsmapper_luac_release_ctx function

The **`fsmapper_luac_release_ctx`** function releases a service context previously created by [`fsmapper_luac_open_ctx`](./fsmapper_luac_open_ctx).

Releasing the [`FSMAPPER_LUAC_CTX`](../data_types) handle informs fsmapper that the calling module no longer needs the associated service resources. Each service context created by [`fsmapper_luac_open_ctx`](./fsmapper_luac_open_ctx) must be released exactly once.

## Syntax

```c
void fsmapper_luac_release_ctx(
    FSMAPPER_LUAC_CTX ctx
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| ctx | [`FSMAPPER_LUAC_CTX`](../data_types) | A service context handle to be released. This value must have been returned by [`fsmapper_luac_open_ctx`](./fsmapper_luac_open_ctx). |

## Return Values

This function does not return a value.

## Remarks

- This function releases the fsmapper-side resources associated with the specified [`FSMAPPER_LUAC_CTX`](../data_types).
- Each service context must be released exactly once. Releasing the same handle more than once results in undefined behavior.
- A service context must be released no later than script termination.
- Lua does not provide an explicit notification when a script terminates. A common pattern is to associate the service context with a userdata object and call this function from the userdata’s [`__gc`](https://www.lua.org/manual/5.4/manual.html#2.5.3) metamethod.
- After this function is called, the released [`FSMAPPER_LUAC_CTX`](../data_types) handle must not be used with any other service API.

## See Also

- [`FSMAPPER_LUAC_CTX`](../data_types)
- [`fsmapper_luac_open_ctx`](./fsmapper_luac_open_ctx) function
- [Basic Functions](../basic)
