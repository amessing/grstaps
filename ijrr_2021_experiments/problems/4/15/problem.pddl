(define (problem ijrr_15)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street18 street27 - street
		building1 building20 building29 - building
	)
	(:init
		(atLocation survivor0 street18)
		(atLocation survivor1 building20)
		(onFire building1)
		(onFire building29)
		(underRubble street27)
		(underRubble building1)
		(needsRepair building1)
		(needsRepair building29)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street27))
		(not (needsRepair building1))
		(not (needsRepair building29))
	))
	(:metric minimize total-time)
)
