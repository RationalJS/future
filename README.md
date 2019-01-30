[![npm](https://img.shields.io/npm/v/reason-future.svg)](https://www.npmjs.com/package/reason-future)
[![Build Status](https://travis-ci.org/RationalJS/future.svg?branch=master)](https://travis-ci.org/RationalJS/future)
[![Coverage Status](https://coveralls.io/repos/github/RationalJS/future/badge.svg?branch=test-coverage)](https://coveralls.io/github/RationalJS/future?branch=test-coverage)

# The Future is Now

Future is a `Js.Promise` alternative. It is written in ReasonML.

Compared to a promise, a future is:

- **Lighter weight** – Only ~25 lines of implementation.
- **Simpler** – Futures only resolve to a single type (as opposed to resolve and reject types), giving you more flexibility on your error handling.
- **More robust** – Futures have sound typing (unlike JS promises).

## Installation

First make sure you have bs-platform `>= 3.1.0`. Then install with npm:

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
futureGreeting->Future.get(x => Js.log("Got value: " ++ x));

/* Alternatively: */

Future.make(resolve => resolve("hi"))
->Future.get(x => Js.log("Got value: " ++ x));
```

`Future.get` only *retrieves* the future value. If you want to **transform** it to a *different* value, then you should use `Future.map`:

```js
/* Shortcut for: let future_A = Future.make(resolve => resolve(99)); */
let future_A = Future.value(99);

let future_B = future_A->Future.map(n => n + 1);


future_A->Future.get(n => Js.log(n)); /* logs: 99 */

future_B->Future.get(n => Js.log(n)); /* logs: 100 */
```

And finally, if you `map` a future and return **another** future, you probably want to `flatMap` instead:

```js
let futureNum = Future.value(50);

let ft_a = futureNum->Future.map(n => Future.value(n + 10));
let ft_b = futureNum->Future.flatMap(n => Future.value(n + 20));

/* ft_a has type future(future(int)) – probably not what you want. */
/* ft_b has type future(int) */
```

## API

Core functions. **Note:** `_` represents the future itself as inserted by `->` (the [fast pipe](https://bucklescript.github.io/docs/en/fast-pipe.html) operator).

- `Future.make(resolver)` - Create a new, potentially-async future.
- `Future.value(x)` - Create a new future with a plain value (synchronous).
- `Future.map(_,fn)` - Transform a future value into another value
- `Future.flatMap(_,fn)` - Transform a future value into another future value
- `Future.get(_,fn)` - Get the value of a future
- `Future.tap(_,fn)` - Do something with the value of a future without changing it. Returns the same future so you can continue using it in a pipeline. Convenient for side effects such as console logging.
- `Future.all(_,fn)` - Turn a list of futures into a future of a list.  Used when you want to wait for a collection of futures to complete before doing something (equivalent to Promise.all in Javascript).

### Belt.Result

Convenience functions when working with a future `Belt.Result`. **Note:** `_` represents the future itself as inserted by `->` (the [fast pipe](https://bucklescript.github.io/docs/en/fast-pipe.html) operator).

**Note 2**: The terms `Result.Ok` and `Result.Error` in this context are expected to be read as `Belt.Result.Ok` and `Belt.Result.Error`.

- `Future.mapOk(_,fn)` - Transform a future value into another value, but only if the value is an `Result.Ok`. Similar to `Promise.prototype.then`
- `Future.mapError(_,fn)` - Transform a future value into another value, but only if the value is an `Result.Error`. Similar to `Promise.prototype.catch`
- `Future.tapOk(_,fn)` - Do something with the value of a future without changing it, but only if the value is a `Belt.Result.Ok`. Returns the same future. Convenience for side effects such as console logging.
- `Future.tapError(_,fn)` - Same as `tapOk` but for `Result.Error`

The following are more situational:

- `Future.flatMapOk(_, fn)` - Transform a future `Result.Ok` into
a future `Result`. Flattens the inner future.
- `Future.flatMapError(_, fn)` - Transform a future `Result.Error` into
a future `Result`. Flattens the inner future.

### FutureJs

Convenience functions for interop with JavaScript land.

- `FutureJs.fromPromise(promise, errorTransformer)`
  - `promise` is the `Js.Promise.t('a)` that will be transformed into a
    `Future.t(Belt.Result.t('a, 'e))`
  - `errorTransformer` allows you to determine how `Js.Promise.error`
    objects will be transformed before they are returned wrapped within
    a `Belt.Result.Error`.  This allows you to implement the error handling
    method which best meets your application's needs.
- `FutureJs.toPromise(future)`
  - `future` is any `Future.t('a)` which is transformed into a
    `Js.Promise.t('a)`. Always resolves, never rejects the promise.
- `FutureJs.resultToPromise(future)`
  - `future` is the `Future.t(Belt.Result('a, 'e))` which is transformed into a
    `Js.Promise.t('a)`. Resolves the promise on Ok result and rejects on Error result.

Example use:


```js
/*
  This error handler is super simple; you will probably want
  to write something more sophisticated in your app.
*/
let handleError = Js.String.make;

somePromiseGetter()
->FutureJs.fromPromise(handleError)
->Future.map(value => Js.log2("It worked!", value))
->Future.mapError(err => Js.log2("uh on", err));
```

See [Composible Error Handling in OCaml][error-handling] for several strategies that you may employ.

## TODO

- [ ] Implement cancellation tokens
- [x] Interop with `Js.Promise`
- [x] `flatMapOk` / `flatMapError` (with [composable error handling](http://keleshev.com/composable-error-handling-in-ocaml))

## Build

```
npm run build
```

## Build + Watch

```
npm run start
```

## Test

```
npm test
```

## Editor
If you use `vscode`, Press `Windows + Shift + B` it will build automatically

[error-handling]: http://keleshev.com/composable-error-handling-in-ocaml
