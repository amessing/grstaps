(define (problem ijrr_0)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street8 street54 - street
		building10 building19 building20 building29 - building
	)
	(:init
		(atLocation survivor0 street54)
		(atLocation survivor1 building19)
		(onFire building20)
		(onFire building29)
		(underRubble street8)
		(underRubble building10)
		(needsRepair building10)
		(needsRepair building20)
		(needsRepair building29)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street8))
		(not (needsRepair building10))
		(not (needsRepair building20))
		(not (needsRepair building29))
	))
	(:metric minimize total-time)
)
