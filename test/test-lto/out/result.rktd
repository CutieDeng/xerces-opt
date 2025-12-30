((file "a.cpp")(type "Container")(field "size")(owned no)(malloc-size ())(reads ())(writes ()))
((file "a.cpp")(type "Container")(field "capacity")(owned no)(malloc-size ())(reads ())(writes ()))
((file "a.cpp")(type "Container")(field "data")(owned yes)(malloc-size ("capacity"))(reads ())(writes (("size" "capacity"))))
((file "b.cpp")(type "Container")(field "data")(owned maybe)(malloc-size ())(reads (("size")))(writes ()))
