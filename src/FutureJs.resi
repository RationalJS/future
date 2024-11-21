let fromPromise: (Js.Promise.t<'a>, Js.Promise.error => 'b) => Future.t<Belt.Result.t<'a, 'b>>
let toPromise: Future.t<'a> => Js.Promise.t<'a>
exception FutureError
let resultToPromise: Future.t<Belt.Result.t<'a, 'b>> => Js.Promise.t<'a>
