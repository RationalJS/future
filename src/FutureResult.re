
let mapOk = (f, future) => future |> Future.map(r => switch(r) {
  | Js.Result.Ok(v) => f(v)
  | Error(e) => Js.Result.Error(e)
});

let flatMapOk = (f, future) => future |> Future.flatMap(r => switch(r) {
  | Js.Result.Ok(v) => f(v)
  | Error(e) => Future.value(Js.Result.Error(e))
});

let mapError = (f, future) => future |> Future.map(r => switch(r) {
  | Js.Result.Error(v) => f(v)
  | Ok(a) => Js.Result.Error(a)
});

let flatMapError = (f, future) => future |> Future.flatMap(r => switch(r) {
  | Js.Result.Error(v) => f(v)
  | Ok(a) => Future.value(Js.Result.Error(a))
});

let tapOk = (f, future) => future |> Future.tap(r => switch(r) {
  | Js.Result.Ok(v) => f(v) |> ignore
  | Error(_) => ()
});

let tapError = (f, future) => future |> Future.tap(r => switch(r) {
  | Js.Result.Error(v) => f(v) |> ignore
  | Ok(_) => ()
});
