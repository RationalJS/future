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

let tapOk = (future, f) => future |. tap(r => switch(r) {
  | Belt.Result.Ok(v) => f(v) |. ignore
  | Error(_) => ()
});

let tapError = (future, f) => future |. tap(r => switch(r) {
  | Belt.Result.Error(v) => f(v) |. ignore
  | Ok(_) => ()
});

let flatMapArray = (xs: array('a), f: 'a => t('b), maxConcurrent) =>
  make(resolve => {
    let result: array(option('b)) =
      xs |. Array.length |. Belt.Array.make(None);
    let xlen = Array.length(xs);
    let numJobs = ref(0);
    let cursor = ref(0);
    let rec pump = () => {
      if (cursor^ == xlen) {
        if (numJobs^ == 0) {
          result |> Array.map(Belt.Option.getExn) |. resolve;
        };
      } else {
        let i = cursor^;
        cursor := cursor^ + 1;
        numJobs := numJobs^ + 1;
        xs[i]
        |. f
        |. get(x => {
             result[i] = Some(x);
             numJobs := numJobs^ - 1;
             pump();
           });
      };
      ();
    };
    for (_ in 1 to min(maxConcurrent, xlen)) {
      pump();
    };
    ();
  });

/** like flatMapArray, but uses static type casts to avoid extra allocations */
let flatMapArrayUnsafe = (xs: array('a), f: 'a => t('b), maxConcurrent) =>
  make(resolve => {
    let result: array('b) =
      xs |. Array.length |. Belt.Array.makeUninitialized |. Obj.magic;
    let xlen = Array.length(xs);
    let numJobs = ref(0);
    let cursor = ref(0);
    let rec pump = () => {
      if (cursor^ == xlen) {
        if (numJobs^ == 0) {
          resolve(result);
        };
      } else {
        let i = cursor^;
        cursor := cursor^ + 1;
        numJobs := numJobs^ + 1;
        xs[i]
        |. f
        |. get(x => {
             result[i] = x;
             numJobs := numJobs^ - 1;
             pump();
           });
      };
      ();
    };
    for (_ in 1 to min(maxConcurrent, xlen)) {
      pump();
    };
    ();
  });
