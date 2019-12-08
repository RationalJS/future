exception TestError(string);

type timeoutId;
[@bs.val] [@bs.val]
external setTimeout: ([@bs.uncurry] (unit => unit), int) => timeoutId = "";

let delay = (ms, f) =>
  Future.make(resolve => setTimeout(() => f() |> resolve, ms) |> ignore);
