type getFn<'a> = ('a => unit) => unit
type executorType = [#none | #trampoline]
type t<'a> = Future(getFn<'a>, executorType)
let trampoline: (unit => unit) => unit
let make: (~executor: executorType=?, ('a => unit) => 'b) => t<'a>
let value: (~executor: executorType=?, 'a) => t<'a>
let map: (t<'a>, 'a => 'b) => t<'b>
let flatMap: (t<'a>, 'a => t<'b>) => t<'b>
let map2: (t<'a>, t<'b>, ('a, 'b) => 'c) => t<'c>
let map3: (t<'a>, t<'b>, t<'c>, ('a, 'b, 'c) => 'd) => t<'d>
let map4: (t<'a>, t<'b>, t<'c>, t<'d>, ('a, 'b, 'c, 'd) => 'e) => t<'e>
let map5: (t<'a>, t<'b>, t<'c>, t<'d>, t<'e>, ('a, 'b, 'c, 'd, 'e) => 'f) => t<'f>
let all: list<t<'a>> => t<list<'a>>
let tap: (t<'a>, 'a => unit) => t<'a>
let get: (t<'a>, 'a => unit) => unit
let mapOk: (t<result<'a, 'b>>, 'a => 'c) => t<result<'c, 'b>>
let mapError: (t<result<'a, 'b>>, 'b => 'c) => t<result<'a, 'c>>
let flatMapOk: (t<result<'a, 'b>>, 'a => t<result<'c, 'b>>) => t<result<'c, 'b>>
let flatMapOkPure: (t<result<'a, 'b>>, 'a => result<'c, 'b>) => t<result<'c, 'b>>
let flatMapError: (t<result<'a, 'b>>, 'b => t<result<'a, 'c>>) => t<result<'a, 'c>>
let mapOk2: (t<result<'a, 'b>>, t<result<'c, 'b>>, ('a, 'c) => 'd) => t<result<'d, 'b>>
let mapOk3: (
  t<result<'a, 'b>>,
  t<result<'c, 'b>>,
  t<result<'d, 'b>>,
  ('a, 'c, 'd) => 'e,
) => t<result<'e, 'b>>
let mapOk4: (
  t<result<'a, 'b>>,
  t<result<'c, 'b>>,
  t<result<'d, 'b>>,
  t<result<'e, 'b>>,
  ('a, 'c, 'd, 'e) => 'f,
) => t<result<'f, 'b>>
let mapOk5: (
  t<result<'a, 'b>>,
  t<result<'c, 'b>>,
  t<result<'d, 'b>>,
  t<result<'e, 'b>>,
  t<result<'f, 'b>>,
  ('a, 'c, 'd, 'e, 'f) => 'g,
) => t<result<'g, 'b>>
let tapOk: (t<result<'a, 'b>>, 'a => 'c) => t<result<'a, 'b>>
let tapError: (t<result<'a, 'b>>, 'b => 'c) => t<result<'a, 'b>>
let delay: (~executor: executorType=?, int, unit => 'a) => t<'a>
let sleep: (~executor: executorType=?, int) => t<unit>
let \">>=": (t<result<'a, 'b>>, 'a => t<result<'c, 'b>>) => t<result<'c, 'b>>
let \">>==": (t<result<'a, 'b>>, 'a => result<'c, 'b>) => t<result<'c, 'b>>
let \"<$>": (t<result<'a, 'b>>, 'a => 'c) => t<result<'c, 'b>>
