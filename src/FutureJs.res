@ocaml.doc("
 * Translate a Promise to a Future(result)
 *
 * errorTransformer: (Promise.error) => 'a
 * - The errorTransformer will provide you with the raw Promise.error
 *   object.  This is done so that you may decide on the appropriate error
 *   handling scheme for your application.
 *   See: http://keleshev.com/composable-error-handling-in-ocaml
 * - A good start is translating the Promise.error to a string.
 *   ```reason
 *   let errorTransformer = (error) =>
 *     String.make(error)
 *     ->(str => /*... do your transforms here */ str);
*    ```
 ")
let fromPromise = (promise, errorTransformer) =>
  Future.make(callback =>
    promise
    ->Promise.then(res => Promise.resolve(ignore(callback(Ok(res)))))
    ->Promise.catch(error =>
      Promise.resolve(
        ignore(callback((transformed => Error(transformed))(errorTransformer(error)))),
      )
    )
  )

let toPromise = future => Promise.make((resolve, _) => future->Future.get(value => resolve(value)))

exception FutureError

let resultToPromise = future =>
  Promise.make((resolve, reject) =>
    future
    ->Future.mapError(_ => FutureError)
    ->Future.map(result =>
      switch result {
      | Ok(result) => resolve(result)
      | Error(error) => reject(error)
      }
    )
    ->ignore
  )
