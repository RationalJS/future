type getFn('a) = ('a => unit, exn => unit) => unit;

type t('a) = Future(getFn('a));

let make = (resolver) => {
  open Belt;

  let successCallbacks = ref([]);
  let failureCallbacks = ref([]);
  let data = ref(None);

  try (
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
    )
  ) {
  | error => data := Some(Error(error))
  };

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


let map = (Future(get), f) => make((resolve, reject) => {
  get(
    result => switch(f(result)) {
      | value => resolve(value)
      | exception error => reject(error)
    },
    reject
  )
});

let flatMap = (Future(get), f) => make((resolve, reject) => {
  get(
    result => switch(f(result)) {
      | Future(get2) => get2(resolve, reject)
      | exception error => reject(error)
    },
    reject
  )
});

let tap = (Future(get), f) => make((resolve, reject) => {
  get(
    result => switch (f(result)) {
      | () => resolve(result)
      | exception error => reject(error)
    },
    reject
  )
});

let catch = (Future(get), f) => make((resolve, reject) => {
  get(
    resolve,
    error => switch (f(error)) {
      | Future(get2) => get2(resolve, reject)
      | exception error => reject(error)
    }
  )
});

let get = (Future(getFn), resolve, reject) => getFn(resolve, reject);

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

let tapOk = (future, f) => future |. tap(r => switch(r) {
  | Belt.Result.Ok(v) => f(v) |. ignore
  | Error(_) => ()
});

let tapError = (future, f) => future |. tap(r => switch(r) {
  | Belt.Result.Error(v) => f(v) |. ignore
  | Ok(_) => ()
});
