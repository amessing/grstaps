(define (problem ijrr_19)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street0 street4 - street
		building19 building28 building29 - building
	)
	(:init
		(atLocation survivor0 street4)
		(atLocation survivor1 building28)
		(onFire building19)
		(underRubble street0)
		(underRubble building29)
		(needsRepair building19)
		(needsRepair building29)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street0))
		(not (needsRepair building19))
		(not (needsRepair building29))
	))
	(:metric minimize total-time)
)
