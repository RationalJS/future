type map_fun('a) = ('a => unit) => unit;

type t('a) = Future(map_fun('a));

let make = (resolver) => {
  let callbacks = ref([]);
  let data = ref(None);

  resolver(result => switch(data^) {
    | None =>
      data := Some(result);
      callbacks^ |. List.rev |. List.iter(cb => cb(result), _);
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
