type getFn('a) = ('a => unit) => unit;
type executorType = [ `none | `trampoline ];
type t('a);
let make: (~executor: executorType=?, ('a => unit) => 'b) => t('a);
let value: (~executor: executorType=?, 'a) => t('a);
let map: (t('a), 'a => 'b) => t('b);
let flatMap: (t('a), 'a => t('b)) => t('b);
let map2: (t('a), t('b), ('a, 'b) => 'c) => t('c);
let map3: (t('a), t('b), t('c), ('a, 'b, 'c) => 'd) => t('d);
let map4: (t('a), t('b), t('c), t('d), ('a, 'b, 'c, 'd) => 'e) => t('e);
let map5:
  (t('a), t('b), t('c), t('d), t('e), ('a, 'b, 'c, 'd, 'e) => 'f) => t('f);
let all: list(t('a)) => t(list('a));
let tap: (t('a), 'a => unit) => t('a);
let get: (t('a), 'a => unit) => unit;
let mapOk: (t(Belt.Result.t('a, 'b)), 'a => 'c) => t(Belt.Result.t('c, 'b));
let mapError:
  (t(Belt.Result.t('a, 'b)), 'b => 'c) => t(Belt.Result.t('a, 'c));
let flatMapOk:
  (t(Belt.Result.t('a, 'b)), 'a => t(Belt.Result.t('c, 'b))) =>
  t(Belt.Result.t('c, 'b));
let flatMapOkPure:
  (t(Belt.Result.t('a, 'b)), 'a => Belt.Result.t('c, 'b)) =>
  t(Belt.Result.t('c, 'b));
let flatMapError:
  (t(Belt.Result.t('a, 'b)), 'b => t(Belt.Result.t('a, 'c))) =>
  t(Belt.Result.t('a, 'c));
let mapOk2:
  (t(Belt.Result.t('a, 'b)), t(Belt.Result.t('c, 'b)), ('a, 'c) => 'd) =>
  t(Belt.Result.t('d, 'b));
let mapOk3:
  (t(Belt.Result.t('a, 'b)), t(Belt.Result.t('c, 'b)),
  t(Belt.Result.t('d, 'b)), ('a, 'c, 'd) => 'e) => t(Belt.Result.t('e, 'b));
let mapOk4:
  (t(Belt.Result.t('a, 'b)), t(Belt.Result.t('c, 'b)),
  t(Belt.Result.t('d, 'b)), t(Belt.Result.t('e, 'b)),
  ('a, 'c, 'd, 'e) => 'f) => t(Belt.Result.t('f, 'b));
let mapOk5:
  (t(Belt.Result.t('a, 'b)), t(Belt.Result.t('c, 'b)),
  t(Belt.Result.t('d, 'b)), t(Belt.Result.t('e, 'b)),
  t(Belt.Result.t('f, 'b)), ('a, 'c, 'd, 'e, 'f) => 'g) =>
  t(Belt.Result.t('g, 'b));
let tapOk: (t(Belt.Result.t('a, 'b)), 'a => 'c) => t(Belt.Result.t('a, 'b));
let tapError:
  (t(Belt.Result.t('a, 'b)), 'b => 'c) => t(Belt.Result.t('a, 'b));
let delay: (~executor: executorType=?, int, unit => 'a) => t('a);
let sleep: (~executor: executorType=?, int) => t(unit);
let ( >>= ):
  (t(Belt.Result.t('a, 'b)), 'a => t(Belt.Result.t('c, 'b))) =>
  t(Belt.Result.t('c, 'b));
let ( >>== ):
  (t(Belt.Result.t('a, 'b)), 'a => Belt.Result.t('c, 'b)) =>
  t(Belt.Result.t('c, 'b));
let ( <$> ): (t(Belt.Result.t('a, 'b)), 'a => 'c) => t(Belt.Result.t('c, 'b));
