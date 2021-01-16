(define (problem ijrr_6)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street34 - street
		building10 building16 building19 building29 - building
	)
	(:init
		(atLocation survivor0 street34)
		(atLocation survivor1 building10)
		(onFire building19)
		(onFire building29)
		(underRubble street34)
		(underRubble building16)
		(needsRepair building16)
		(needsRepair building19)
		(needsRepair building29)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street34))
		(not (needsRepair building16))
		(not (needsRepair building19))
		(not (needsRepair building29))
	))
	(:metric minimize total-time)
)
