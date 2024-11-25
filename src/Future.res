type getFn<'a> = ('a => unit) => unit

type executorType = [#none | #trampoline]

type t<'a> = Future(getFn<'a>, executorType)

let trampoline = {
  let running = ref(false)
  let callbacks = []
  let rec runLoop = callback => {
    callback()
    switch Js.Array.pop(callbacks) {
    | None => ()
    | Some(callback) => runLoop(callback)
    }
  }
  callback =>
    if running.contents {
      ignore(Js.Array.unshift(callback, callbacks))
    } else {
      running := true
      runLoop(callback)
      running := false
    }
}

let make = (~executor: executorType=#none, resolver) => {
  let callbacks = ref(list{})
  let data = ref(None)

  let runCallback = switch executor {
  | #none => (result, cb) => cb(result)
  | #trampoline => (result, cb) => trampoline(() => cb(result))
  }

  resolver(result =>
    switch data.contents {
    | None =>
      data := Some(result)
      callbacks.contents->List.reverse->List.forEach(cb => runCallback(result, cb))
      /* Clean up memory usage */
      callbacks := list{}
    | Some(_) => () /* Do nothing; theoretically not possible */
    }
  )->ignore

  Future(
    resolve =>
      switch data.contents {
      | Some(result) => runCallback(result, resolve)
      | None => callbacks := list{resolve, ...callbacks.contents}
      },
    executor,
  )
}

let value = (~executor: executorType=#none, x) => make(~executor, resolve => resolve(x))

let map = (Future(get, executor), f) =>
  make(~executor, resolve => get(result => resolve(f(result))))

let flatMap = (Future(get, executor), f) =>
  make(~executor, resolve =>
    get(result => {
      let Future(get2, _) = f(result)
      get2(resolve)
    })
  )

let map2 = (fa, fb, f) => flatMap(fa, a => map(fb, b => f(a, b)))

let map3 = (fa, fb, fc, f) => map2(map2(fa, fb, (a, b) => (a, b)), fc, ((a, b), c) => f(a, b, c))

let map4 = (fa, fb, fc, fd, f) =>
  map2(map3(fa, fb, fc, (a, b, c) => (a, b, c)), fd, ((a, b, c), d) => f(a, b, c, d))

let map5 = (fa, fb, fc, fd, fe, f) =>
  map2(map4(fa, fb, fc, fd, (a, b, c, d) => (a, b, c, d)), fe, ((a, b, c, d), e) =>
    f(a, b, c, d, e)
  )

let rec all = futures =>
  switch futures {
  | list{head, ...tail} =>
    all(tail)->flatMap(tailResult => head->map(headResult => list{headResult, ...tailResult}))
  | list{} => value(list{})
  }

let tap = (Future(get, _) as future, f) => {
  get(f)
  future
}

let get = (Future(getFn, _), f) => getFn(f)

/**
 * Future Result convenience functions,
 * for working with a type Future.t( result('a,'b) )
 */
let mapOk: (t<result<'a, 'b>>, 'a => 'c) => t<result<'c, 'b>> = (future, f) =>
  future->map(r => Result.map(r, f))

let mapError = (future, f) =>
  future->map(r =>
    switch r {
    | Error(v) => Error(f(v))
    | Ok(a) => Ok(a)
    }
  )

let flatMapOk = (future, f) =>
  future->flatMap(r =>
    switch r {
    | Ok(v) => f(v)
    | Error(e) => value(Error(e))
    }
  )

let flatMapOkPure = (fut, f) => fut->flatMapOk(result => value(result->f))

let flatMapError = (future, f) =>
  future->flatMap(r =>
    switch r {
    | Ok(v) => value(Ok(v))
    | Error(e) => f(e)
    }
  )

let mapOk2 = (fa, fb, f) => flatMapOk(fa, a => mapOk(fb, b => f(a, b)))

let mapOk3 = (fa, fb, fc, f) =>
  mapOk2(mapOk2(fa, fb, (a, b) => (a, b)), fc, ((a, b), c) => f(a, b, c))

let mapOk4 = (fa, fb, fc, fd, f) =>
  mapOk2(mapOk3(fa, fb, fc, (a, b, c) => (a, b, c)), fd, ((a, b, c), d) => f(a, b, c, d))

let mapOk5 = (fa, fb, fc, fd, fe, f) =>
  mapOk2(mapOk4(fa, fb, fc, fd, (a, b, c, d) => (a, b, c, d)), fe, ((a, b, c, d), e) =>
    f(a, b, c, d, e)
  )

let tapOk = (future, f) =>
  future->tap(r =>
    switch r {
    | Ok(v) => f(v)->ignore
    | Error(_) => ()
    }
  )

let tapError = (future, f) =>
  future->tap(r =>
    switch r {
    | Error(v) => f(v)->ignore
    | Ok(_) => ()
    }
  )

let delay = (~executor=?, ms, f) =>
  make(~executor?, resolve => Js.Global.setTimeout(() => resolve(f()), ms)->ignore)

let sleep = (~executor=?, ms) => delay(~executor?, ms, () => ())

let \">>=" = flatMapOk
let \">>==" = flatMapOkPure
let \"<$>" = mapOk
