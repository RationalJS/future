type getFn('a) = ('a => unit) => unit;

type executorType = [ | `none | `trampoline];

type status('value) =
  | Pending(array('value => unit))
  | Cancelled
  | Done('value);

[@unboxed]
type cancellationToken =
  | Cancel(unit => unit);

type cancelFunction = unit => unit;

type futureCallback('value) = 'value => unit;

type resolve('value) = 'value => unit;

type setup('value) = resolve('value) => option(cancelFunction);

[@unboxed]
type t('value) =
  | Future(futureCallback('value) => cancellationToken, executorType);

let noop = _ => ();

let trampoline = {
  let running = ref(false);
  let callbacks = [||];
  let rec runLoop = callback => {
    callback();
    switch (Js.Array.pop(callbacks)) {
    | None => ()
    | Some(callback) => runLoop(callback)
    };
  };
  callback =>
    if (running^) {
      Js.Array.unshift(callback, callbacks) |> ignore;
    } else {
      running := true;
      runLoop(callback);
      running := false;
    };
};

let make =
    (~executor: executorType=`none, resolver: setup('value)): t('value) => {
  let status = ref(Pending([||]));
  let runCallback =
    switch (executor) {
    | `none => ((result, cb) => cb(result))
    | `trampoline => ((result, cb) => trampoline(() => cb(result)))
    };
  let maybeCancel =
    resolver(value =>
      switch (status.contents) {
      | Pending(subscriptions) =>
        status := Done(value);
        subscriptions->Js.Array2.forEach(cb => runCallback(value, cb));
      | Cancelled
      | Done(_) => ()
      }
    );
  let futureGet = cb => {
    switch (status.contents) {
    | Done(value) =>
      runCallback(value, cb);
      Cancel(noop);
    | Cancelled => Cancel(noop)
    | Pending(subscriptions) =>
      let _ = subscriptions->Js.Array2.push(cb);
      Cancel(
        () => {
          switch (maybeCancel) {
          | Some(cancel) => cancel()
          | None => ()
          };
          status := Cancelled;
        },
      );
    };
  };
  Future(futureGet, executor);
};

let cancel = (Future(getFunc, _)) => {
  let Cancel(cancel) = getFunc(_ => ());
  cancel();
};

let value = (~executor: executorType=`none, x) =>
  make(
    ~executor,
    resolve => {
      resolve(x);
      None;
    },
  );

let map = (Future(get, executor), ~propagateCancel=true, f) =>
  make(
    ~executor,
    resolve => {
      let Cancel(cancel) = get(value => resolve(f(value)));
      if (propagateCancel) {
        Some(() => cancel());
      } else {
        None;
      };
    },
  );

let flatMap = (Future(get, executor), ~propagateCancel=true, f) =>
  make(
    ~executor,
    resolve => {
      let Cancel(cancel) =
        get(val1 => {
          let Future(get2, _) = f(val1);
          let _ = get2(val2 => resolve(val2));
          ();
        });
      if (propagateCancel) {
        Some(() => cancel());
      } else {
        None;
      };
    },
  );

let map2 = (fa, fb, ~propagateCancel=?, f) =>
  flatMap(fa, ~propagateCancel?, a =>
    map(fb, ~propagateCancel?, b => f(a, b))
  );

let map3 = (fa, fb, fc, ~propagateCancel=?, f) =>
  map2(map2(fa, fb, ~propagateCancel?, f), fc, ~propagateCancel?, v => v);

let map4 = (fa, fb, fc, fd, ~propagateCancel=?, f) =>
  map3(map2(fa, fb, ~propagateCancel?, f), fc, fd, ~propagateCancel?, v => v);

let map5 = (fa, fb, fc, fd, fe, ~propagateCancel=?, f) =>
  map4(map2(fa, fb, ~propagateCancel?, f), fc, fd, fe, ~propagateCancel?, v =>
    v
  );

let rec all = futures =>
  switch (futures) {
  | [head, ...tail] =>
    all(tail)
    ->flatMap(tailResult =>
        head->map(headResult => [headResult, ...tailResult])
      )
  | [] => value([])
  };

let tap = (Future(get, _) as future, f) => {
  let _ = get(f);
  future;
};

let get = (Future(getFn, _), f) => {
  let _ = getFn(f);
  ();
};

/* *
 * Future Belt.Result convenience functions,
 * for working with a type Future.t( Belt.Result.t('a,'b) )
 */
let mapOk = (future, f) => future->map(r => Belt.Result.map(r, f));

let mapError = (future, f) =>
  future->map(r =>
    switch (r) {
    | Belt.Result.Error(v) => Belt.Result.Error(f(v))
    | Ok(a) => Belt.Result.Ok(a)
    }
  );

let flatMapOk = (future, f) =>
  future->flatMap(r =>
    switch (r) {
    | Belt.Result.Ok(v) => f(v)
    | Belt.Result.Error(e) => value(Belt.Result.Error(e))
    }
  );

let flatMapError = (future, f) =>
  future->flatMap(r =>
    switch (r) {
    | Belt.Result.Ok(v) => value(Belt.Result.Ok(v))
    | Belt.Result.Error(e) => f(e)
    }
  );

let mapOk2 = (fa, fb, f) => flatMapOk(fa, a => mapOk(fb, b => f(a, b)));

let mapOk3 = (fa, fb, fc, f) => mapOk2(mapOk2(fa, fb, f), fc, v => v);

let mapOk4 = (fa, fb, fc, fd, f) =>
  mapOk3(mapOk2(fa, fb, f), fc, fd, v => v);

let mapOk5 = (fa, fb, fc, fd, fe, f) =>
  mapOk4(mapOk2(fa, fb, f), fc, fd, fe, v => v);

let tapOk = (future, f) =>
  future->tap(r =>
    switch (r) {
    | Belt.Result.Ok(v) => f(v)->ignore
    | Error(_) => ()
    }
  );

let tapError = (future, f) =>
  future->tap(r =>
    switch (r) {
    | Belt.Result.Error(v) => f(v)->ignore
    | Ok(_) => ()
    }
  );

let delay = (~executor=?, ms, f) =>
  make(
    ~executor?,
    resolve => {
      let timeoutId = Js.Global.setTimeout(() => f() |> resolve, ms);
      Some(() => Js.Global.clearTimeout(timeoutId));
    },
  );

let sleep = (~executor=?, ms) => delay(~executor?, ms, () => ());

let (>>=) = flatMapOk;
let (<$>) = mapOk;
