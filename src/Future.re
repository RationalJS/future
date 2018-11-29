type getFn('a) = ('a => unit) => unit;

type t('a) = Future(getFn('a));

let make = (resolver) => {
  let callbacks = ref([]);
  let data = ref(None);

  resolver(result => switch(data^) {
    | None =>
      data := Some(result);
      callbacks^ |. Belt.List.reverse |. Belt.List.forEach(cb => cb(result));
      /* Clean up memory usage */
      callbacks := []
    | Some(_) =>
      () /* Do nothing; theoretically not possible */
  });

  Future(resolve => switch(data^) {
    | Some(result) => resolve(result)
    | None => callbacks := [resolve, ...callbacks^]
  })
};

let value = (x) => make(resolve => resolve(x));


let map = (Future(get), f) => make(resolve => {
  get(result => resolve(f(result)))
});

let flatMap = (Future(get), f) => make(resolve => {
  get(result => {
    let Future(get2) = f(result);
    get2(resolve)
  })
});

let map2 = (fa, fb, f) =>
  flatMap(fa, a => map(fb, b => f(a, b)));

let map3 = (fa, fb, fc, f) =>
  map2(map2(fa, fb, f), fc, v => v);

let map4 = (fa, fb, fc, fd, f) =>
  map3(map2(fa, fb, f), fc, fd, v => v);

let map5 = (fa, fb, fc, fd, fe, f) =>
  map4(map2(fa, fb, f), fc, fd, fe, v => v);

let tap = (Future(get) as future, f) => {
  get(f);
  future
};

let get = (Future(getFn), f) => getFn(f);

/* *
 * Future Belt.Result convenience functions,
 * for working with a type Future.t( Belt.Result.t('a,'b) )
 */
let mapOk = (future, f) => future |. map(r => Belt.Result.map(r,f));

let mapError = (future, f) => future |. map(r => switch(r) {
  | Belt.Result.Error(v) => Belt.Result.Error(f(v))
  | Ok(a) => Belt.Result.Ok(a)
});

let flatMapOk = (future, f) => future |. flatMap(r =>
  switch(r) {
  | Belt.Result.Ok(v) => f(v)
  | Belt.Result.Error(e) => value(Belt.Result.Error(e))
  });

let flatMapError = (future, f) => future |. flatMap(r =>
  switch(r) {
  | Belt.Result.Ok(v) => value(Belt.Result.Ok(v))
  | Belt.Result.Error(e) => f(e)
  });

let mapOk2 = (fa, fb, f) =>
  flatMapOk(fa, a => mapOk(fb, b => f(a, b)));

let mapOk3 = (fa, fb, fc, f) =>
  mapOk2(mapOk2(fa, fb, f), fc, v => v);

let mapOk4 = (fa, fb, fc, fd, f) =>
  mapOk3(mapOk2(fa, fb, f), fc, fd, v => v);

let mapOk5 = (fa, fb, fc, fd, fe, f) =>
  mapOk4(mapOk2(fa, fb, f), fc, fd, fe, v => v);

let tapOk = (future, f) => future |. tap(r => switch(r) {
  | Belt.Result.Ok(v) => f(v) |. ignore
  | Error(_) => ()
});

let tapError = (future, f) => future |. tap(r => switch(r) {
  | Belt.Result.Error(v) => f(v) |. ignore
  | Ok(_) => ()
});

let (>>=) = flatMapOk;
let (<$>) = mapOk;
