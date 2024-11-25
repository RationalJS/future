let fromPromise: (RescriptCore.Promise.t<'a>, exn => 'b) => Future.t<result<'a, 'b>>
let toPromise: Future.t<'a> => RescriptCore.Promise.t<'a>
type exn +=  FutureError
let resultToPromise: Future.t<result<'a, 'b>> => RescriptCore.Promise.t<'a>
