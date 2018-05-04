# The Future is Now

Future is a `Js.Promise` alternative. It is written in ReasonML.

Compared to a promise, a future is:

- **Lighter weight** – Only ~25 lines of implementation.
- **Simpler** – Futures only resolve to a single type (as opposed to resolve and reject types), giving you more flexibility on your error handling.
- **More rubust** – Futures have sound typing (unlike JS promises).

## Installation

```
$ npm install --save rational-future
```

Then add `"rational-future"` to your `bsconfig.json` dev dependencies:

```
{
  ...
  "bs-dependencies": [
    "rational-future"
  ]
}
```

## Basic Usage

To create a task, use `Future.make`. It uses a `resolve` function similar to a promise (but no `reject`, however):

```re
let futureGreeting = Future.make(resolve => resolve("hi"));
```

To get the value of a future, use `Future.get`:

```re
let futureGreeting = Future.make(resolve => resolve("hi"));
futureGreeting
|> Future.get(x => Js.log("Got value: " ++ x));

/* alternatively: */
Future.make(resolve => resolve("hi"))
|> Future.get(x => Js.log("Got value: " ++ x));
```

`Future.get` only retrieves the future value. If you want to **transform** it to a *different* value, then you should use `Future.map`:

```re
let futureNum = Future.value(99);
/* Shortcut for: let futureNum = Future.make(resolve => resolve(99)); */

let secondFuture = futureNum |> Future.map(n => n + 1);


futureNum
|> Future.get(n => Js.log(n)); /* logs: 99 */

secondFuture
|> Future.get(n => Js.log(n)); /* logs: 100 */
```

And finally, if you `map` a future and return **another** future, you probably want to `flatMap` instead:

```re
let futureNum = Future.value(50);

let ft_a = futureNum.map(n => Future.value(n + 10));
let ft_b = futureNum.flatMap(n => Future.value(n + 20));

/* ft_a has type future(future(int)) – probably not what you want. */
/* ft_b has type future(int) */
```

## API

- `Future.make(resolver)` - Create a new, potentially-async future.
- `Future.value(x)` - Create a new future with a plain value (synchronous).
- `Future.map(f)` - Transform a future value into another value
- `Future.flatMap(f)` - Transform a future value into another future value
- `Future.get(f)` - Get the value of a future
- `Future.tap(f)` - Do something with the value of a future without changing it. Returns the same future so you can continue using it in a pipeline. Convenient for logging.

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
