exception TestError(string);

type timeoutId;
[@bs.val] [@bs.val]
external setTimeout: ([@bs.uncurry] (unit => unit), int) => timeoutId = "";

let delay = (~executor=`none, ms, f) =>
  Future.make(~executor, resolve => setTimeout(() => f() |> resolve, ms) |> ignore);
