type getFn('a) = ('a => unit, exn => unit) => unit;

type t('a) = Future(getFn('a));

let make = (resolver) => {
  open Belt;

  let successCallbacks = ref([]);
  let failureCallbacks = ref([]);
  let data = ref(None);

  resolver(
    result => switch(data^) {
      | None =>
        data := Some(Result.Ok(result));
        successCallbacks^ |. List.reverse |. List.forEach(cb => cb(result));
        /* Clean up memory usage */
        successCallbacks := [];
        failureCallbacks := []
      | Some(_) =>
        () /* Do nothing; theoretically not possible */
    },
    error => switch (data^) {
      | None =>
        data := Some(Result.Error(error));
        failureCallbacks^ |. List.reverse |. List.forEach(cb => cb(error));

        successCallbacks := [];
        failureCallbacks := []
      | Some(_) =>
        ()
    }
  );

  Future((resolve, reject) => switch(data^) {
    | Some(Ok(result)) => resolve(result)
    | Some(Error(error)) => reject(error)
    | None =>
      successCallbacks := [resolve, ...successCallbacks^];
      failureCallbacks := [reject, ...failureCallbacks^];
  })
};

let value = (x) => make((resolve, _reject) => resolve(x));

let error = (e) => make((_resolve, reject) => reject(e));


let map = (Future(get), f) => make(resolve => {
  get(result => resolve(f(result)))
});

let flatMap = (Future(get), f) => make((resolve, reject) => {
  get(
    result => {
      let Future(get2) = f(result);
      get2(resolve, reject)
    },
    reject
  )
});

let tap = (Future(get) as future, f) => {
  get(f, _error => ());
  future
};

let catch = (Future(get), f) => make((resolve, reject) => {
  get(
    resolve,
    error => {
      let Future(get2) = f(error);
      get2(resolve, reject)
    }
  )
});

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


let tapOk = (future, f) => future |. tap(r => switch(r) {
  | Belt.Result.Ok(v) => f(v) |. ignore
  | Error(_) => ()
});

let tapError = (future, f) => future |. tap(r => switch(r) {
  | Belt.Result.Error(v) => f(v) |. ignore
  | Ok(_) => ()
});
