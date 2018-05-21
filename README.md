# The Future is Now

Future is a `Js.Promise` alternative. It is written in ReasonML.

Compared to a promise, a future is:

- **Lighter weight** – Only ~25 lines of implementation.
- **Simpler** – Futures only resolve to a single type (as opposed to resolve and reject types), giving you more flexibility on your error handling.
- **More rubust** – Futures have sound typing (unlike JS promises).

## Installation

```
$ npm install --save reason-future
```

Then add `"reason-future"` to your `bsconfig.json` dev dependencies:

```
{
  ...
  "bs-dependencies": [
    "reason-future"
  ]
}
```

## Basic Usage

To create a task, use `Future.make`. It provides a single `resolve` function, like a promise with no `reject`:

```js
let futureGreeting = Future.make(resolve => resolve("hi"));
```

To get the value of a future, use `Future.get`:

```js
let futureGreeting = Future.make(resolve => resolve("hi"));
futureGreeting
|. Future.get(x => Js.log("Got value: " ++ x));

/* Alternatively: */

Future.make(resolve => resolve("hi"))
|. Future.get(x => Js.log("Got value: " ++ x));
```

`Future.get` only *retrieves* the future value. If you want to **transform** it to a *different* value, then you should use `Future.map`:

```js
/* Shortcut for: let future_A = Future.make(resolve => resolve(99)); */
let future_A = Future.value(99);

let future_B = future_A |. Future.map(n => n + 1);


future_A
|. Future.get(n => Js.log(n)); /* logs: 99 */

future_B
|. Future.get(n => Js.log(n)); /* logs: 100 */
```

And finally, if you `map` a future and return **another** future, you probably want to `flatMap` instead:

```js
let futureNum = Future.value(50);

let ft_a = futureNum |. Future.map(n => Future.value(n + 10));
let ft_b = futureNum |. Future.flatMap(n => Future.value(n + 20));

/* ft_a has type future(future(int)) – probably not what you want. */
/* ft_b has type future(int) */
```

## API

Core functions. **Note:** `_` represents the future itself as inserted by `|.` (the [fast pipe](https://bucklescript.github.io/docs/en/fast-pipe.html) operator).

- `Future.make(resolver)` - Create a new, potentially-async future.
- `Future.value(x)` - Create a new future with a plain value (synchronous).
- `Future.map(_,fn)` - Transform a future value into another value
- `Future.flatMap(_,fn)` - Transform a future value into another future value
- `Future.get(_,fn)` - Get the value of a future
- `Future.tap(_,fn)` - Do something with the value of a future without changing it. Returns the same future so you can continue using it in a pipeline. Convenient for side effects such as console logging.

### FutureResult

Convenience functions when working with a future `Js.Result`. **Note:** `_` represents the future itself as inserted by `|.` (the [fast pipe](https://bucklescript.github.io/docs/en/fast-pipe.html) operator).

- `FutureResult.mapOk(_,fn)` - Transform a future value into another value, but only if the value is a `Js.Result.Ok`. Similar to `Promise.prototype.then`
- `FutureResult.mapError(_,fn)` - Transform a future value into another value, but only if the value is a `Js.Result.Error`. Similar to `Promise.prototype.catch`
- `FutureResult.flatMapOk(_,fn)` - Same as `mapOk` but flattens.
- `FutureResult.flatMapError(_,fn)` - Same as `mapError` but flattens.

## TODO

- Implement cancellation tokens
- Interop with `Js.Promise`

## Build
```
npm run build
```

## Build + Watch

```
npm run start
```

## Editor
If you use `vscode`, Press `Windows + Shift + B` it will build automatically
