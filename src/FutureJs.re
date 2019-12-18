/**
 * Translate a Js.Promise to a Future(Belt.Result.t)
 *
 * errorTransformer: (Js.Promise.error) => 'a
 * - The errorTransformer will provide you with the raw Js.Promise.error
 *   object.  This is done so that you may decide on the appropriate error
 *   handling scheme for your application.
 *   See: http://keleshev.com/composable-error-handling-in-ocaml
 * - A good start is translating the Js.Promise.error to a string.
 *   ```reason
 *   let errorTransformer = (error) =>
 *     Js.String.make(error)
 *     ->(str => /*... do your transforms here */ str);
*    ```
 */
let fromPromise = (promise, errorTransformer) =>
  Future.make(callback =>
    promise
    |> Js.Promise.then_(res =>
         Belt.Result.Ok(res) |> callback |> ignore |> Js.Promise.resolve
       )
    |> Js.Promise.catch(error =>
         errorTransformer(error)
         |> (transformed => Belt.Result.Error(transformed))
         |> callback
         |> ignore
         |> Js.Promise.resolve
       )
    |> ignore
  );

let toPromise = future =>
  Js.Promise.make((~resolve, ~reject as _) =>
    future->Future.get(value => resolve(. value))
  );

exception FutureError;

let resultToPromise = future =>
  Js.Promise.make((~resolve, ~reject) =>
    future
    ->Future.mapError(_ => FutureError)
    ->Future.map(result =>
        switch (result) {
        | Belt.Result.Ok(result) => resolve(. result)
        | Belt.Result.Error(error) => reject(. error)
        }
      )
    ->ignore
  );