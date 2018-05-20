
let mapOk = (future, f) => future |. Future.map(r => switch(r) {
  | Js.Result.Ok(v) => f(v)
  | Error(e) => Js.Result.Error(e)
});

let flatMapOk = (future, f) => future |. Future.flatMap(r => switch(r) {
  | Js.Result.Ok(v) => f(v)
  | Error(e) => Future.value(Js.Result.Error(e))
});

let mapError = (future, f) => future |. Future.map(r => switch(r) {
  | Js.Result.Error(v) => f(v)
  | Ok(a) => Js.Result.Error(a)
});

let flatMapError = (future, f) => future |. Future.flatMap(r => switch(r) {
  | Js.Result.Error(v) => f(v)
  | Ok(a) => Future.value(Js.Result.Error(a))
});

let tapOk = (future, f) => future |. Future.tap(r => switch(r) {
  | Js.Result.Ok(v) => f(v) |. ignore
  | Error(_) => ()
});

let tapError = (future, f) => future |. Future.tap(r => switch(r) {
  | Js.Result.Error(v) => f(v) |. ignore
  | Ok(_) => ()
});
